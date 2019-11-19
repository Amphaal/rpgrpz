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
#include "src/shared/hints/AtomsStorage.h"

#include "src/ui/atomEditor/_base/NoEditorMessageWidget.hpp"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"

#include "src/ui/atomEditor/editors/generic/AtomSliderEditor.hpp"
#include "src/ui/atomEditor/editors/generic/NonLinearAtomSliderEditor.hpp"
#include "src/ui/atomEditor/editors/generic/AtomTextEditor.hpp"
#include "src/ui/atomEditor/editors/generic/AtomShortTextEditor.hpp"

#include "src/ui/atomEditor/editors/specific/BrushToolEditor.hpp"
#include "src/ui/atomEditor/editors/specific/CharacterPickerEditor.hpp"
#include "src/ui/atomEditor/editors/specific/NPCAttitudeEditor.hpp"
#include "src/ui/atomEditor/editors/specific/NPCHealthEditor.hpp"

#include "src/shared/async-ui/AlterationActor.hpp"

class AtomEditor : public QGroupBox, public AlterationActor {

    Q_OBJECT

    public:
        AtomEditor(QWidget* parent = nullptr);
        void buildEditor(const AtomsSelectionDescriptor &atomsSelectionDescr);
        void resetParams();
        bool hasVisibleEditors();
        AtomsSelectionDescriptor currentSelectionDescriptor();
    
    signals:
        void requiresPreview(const AtomsSelectionDescriptor &selectionDescriptor, const RPZAtom::Parameter &parameter, const QVariant &value);

    private:
        static inline QHash<AtomSubEditor::EditMode, QString> _strEM {
            { AtomSubEditor::EditMode::None, QT_TR_NOOP("Nothing to modify") },
            { AtomSubEditor::EditMode::Template, QT_TR_NOOP("Template modification") },
            { AtomSubEditor::EditMode::Selection, QT_TR_NOOP("Selection modification") }
        };

        AtomsSelectionDescriptor _currentSelectionDescr;

        QMap<RPZAtom::Parameter, AtomSubEditor*> _editorsByParam;
        void _addEditor(AtomSubEditor* editor);

        NoEditorMessageWidget* _noEditorMsgWidget = nullptr;

        QList<AtomSubEditor*> _visibleEditors;

        RPZAtom::Updates _findDefaultValuesToBind();
        void _createEditorsFromAtomParameters();

        void _onPreviewRequested(const RPZAtom::Parameter &parameter, const QVariant &value);
        void _emitPayload(const RPZAtom::Updates &changesToEmit);

        AtomSubEditor::EditMode _currentEditMode = AtomSubEditor::EditMode::None;
        void _updateEditMode();

        void _mustShowBrushPenWidthEditor(const RPZAtom::Updates &updatedValues);
};
