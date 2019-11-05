#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <QGroupBox>

#include <QPushButton>
#include <QLabel>
#include <QSlider>

#include <QtMath>

#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/RPZAtom.h"

#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/atomEditor/editors/AtomSliderEditor.h"
#include "src/ui/atomEditor/editors/NonLinearAtomSliderEditor.hpp"
#include "src/ui/atomEditor/editors/specific/BrushToolEditor.hpp"
#include "src/ui/atomEditor/editors/AtomTextEditor.hpp"
#include "src/ui/atomEditor/editors/AtomShortTextEditor.hpp"
#include "src/ui/atomEditor/_base/NoEditorMessageWidget.hpp"

#include "src/shared/async-ui/AlterationActor.hpp"

class AtomEditor : public QGroupBox, public AlterationActor {

    Q_OBJECT

    public:
        enum class EditMode { None, Template, Selection };
        Q_ENUM(EditMode)

        AtomEditor(QWidget* parent = nullptr);
        void buildEditor(const AtomsSelectionDescriptor &atomsSelectionDescr);
        void resetParams();
        bool hasVisibleEditors();
        AtomsSelectionDescriptor currentSelectionDescriptor();
    
    signals:
        void requiresPreview(const AtomsSelectionDescriptor &selectionDescriptor, const RPZAtom::Parameter &parameter, const QVariant &value);

    private:
        static inline QHash<EditMode, QString> _strEM {
            { EditMode::None, QT_TR_NOOP("Nothing to modify") },
            { EditMode::Template, QT_TR_NOOP("Template modification") },
            { EditMode::Selection, QT_TR_NOOP("Selection modification") }
        };

        AtomsSelectionDescriptor _currentSelectionDescr;

        QMap<RPZAtom::Parameter, AtomSubEditor*> _editorsByParam;
        NoEditorMessageWidget* _noEditorMsgWidget = nullptr;

        QList<RPZAtom::Parameter> _visibleEditors;

        RPZAtom::Updates _findDefaultValuesToBind();
        void _createEditorsFromAtomParameters();

        void _onPreviewRequested(const RPZAtom::Parameter &parameter, const QVariant &value);
        void _emitPayload(const RPZAtom::Updates &changesToEmit);

        EditMode _currentEditMode = EditMode::None;
        void _updateEditMode();

        void _mustShowBrushPenWidthEditor(const RPZAtom::Updates &updatedValues);
};

inline uint qHash(const AtomEditor::EditMode &key, uint seed = 0) {return uint(key) ^ seed;}
