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
#include "src/ui/atomEditor/editors/BrushToolEditor.hpp"
#include "src/ui/atomEditor/editors/AtomTextEditor.hpp"
#include "src/ui/atomEditor/_base/NoEditorMessageWidget.hpp"

#include "src/shared/async-ui/AlterationActor.hpp"

class AtomEditor : public QGroupBox, public AlterationActor {

    Q_OBJECT

    public:
        enum class EditMode { None, Template, Selection };

        AtomEditor(QWidget* parent = nullptr);
        void buildEditor(const AtomsSelectionDescriptor &atomsSelectionDescr);
        void resetParams();
        bool hasVisibleEditors();
        AtomsSelectionDescriptor currentSelectionDescriptor();
    
    signals:
        void requiresPreview(const AtomsSelectionDescriptor &selectionDescriptor, const AtomParameter &parameter, const QVariant &value);

    private:
        static inline QHash<EditMode, QString> _strEM {
            { EditMode::None, QT_TR_NOOP("Nothing to modify") },
            { EditMode::Template, QT_TR_NOOP("Template modification") },
            { EditMode::Selection, QT_TR_NOOP("Selection modification") }
        };

        AtomsSelectionDescriptor _currentSelectionDescr;

        QMap<AtomParameter, AtomSubEditor*> _editorsByParam;
        NoEditorMessageWidget* _noEditorMsgWidget = nullptr;

        QList<AtomParameter> _visibleEditors;

        AtomUpdates _findDefaultValuesToBind();
        void _createEditorsFromAtomParameters();

        void _onPreviewRequested(const AtomParameter &parameter, const QVariant &value);
        void _emitPayload(const AtomUpdates &changesToEmit);
        void _emitPayloadCB(const AtomParameter &parameter, const QVariant &value);

        EditMode _currentEditMode = EditMode::None;
        void _updateEditMode();

        void _mustShowBrushPenWidthEditor(const AtomParameter &paramToCheck, const QVariant &defaultValue);
};