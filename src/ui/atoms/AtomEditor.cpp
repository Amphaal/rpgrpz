#include "AtomEditor.h"

AtomEditor::AtomEditor(QWidget* parent) : QGroupBox(_strEM[None], parent) {

    this->setAlignment(Qt::AlignHCenter);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    this->setLayout(new QVBoxLayout);

    //create params editors
    this->_createEditorsFromAtomParameters();
}


void AtomEditor::buildEditor(const QVector<RPZAtom*> &atomsToBuildFrom) {
    
    //modify atom list
    this->_atoms = atomsToBuildFrom;
    this->_RPZAtomIds.clear();
    for(auto atom : this->_atoms) this->_RPZAtomIds.append(atom->id());
    
    //clear editors
    this->_visibleEditors.clear();

    //update edit mode
    this->_updateEditMode();

    //fetch parameter editors to display
    auto toDisplay = this->_findDefaultValuesToBind();

    //load those who need to be displayed
    for(auto i = toDisplay.begin(); i != toDisplay.end(); ++i) {
        
        //prepare
        auto param = i.key();
        auto editor = this->_editorsByParam[param];
        if(!editor) continue;

        //load template, and display them
        editor->loadTemplate(this->_atoms, i.value());

        //add to the visible editors list
        this->_visibleEditors.append(param);

    }

    //hide the others
    auto toHide = _editorsByParam.keys().toSet().subtract(
        toDisplay.keys().toSet()
    );
    for(auto i : toHide) {
        auto editor = this->_editorsByParam[i];
        if(!editor) continue;
        editor->setVisible(false);
    }
}

//
//
//

void AtomEditor::resetParams() {

    //reset displayed params
        AtomUpdates changes;
        for(auto param : this->_visibleEditors) {
            changes.insert(param, QVariant());
        }

        MetadataChangedPayload payload(this->_RPZAtomIds, changes);
        this->_emitPayload(payload);

}

void AtomEditor::_createEditorsFromAtomParameters() {

    this->_editorsByParam[AtomParameter::BrushStyle] = new BrushToolEditor;
    this->_editorsByParam[AtomParameter::BrushPenWidth] = new AtomSliderEditor(AtomParameter::BrushPenWidth, 1, 500);

    this->_editorsByParam[AtomParameter::Rotation] = new AtomSliderEditor(AtomParameter::Rotation, 0, 359);
    this->_editorsByParam[AtomParameter::Scale] = new NonLinearAtomSliderEditor(AtomParameter::Scale, 1, 1000);
    
    this->_editorsByParam[AtomParameter::AssetRotation] = new AtomSliderEditor(AtomParameter::AssetRotation, 0, 359);
    this->_editorsByParam[AtomParameter::AssetScale] = new NonLinearAtomSliderEditor(AtomParameter::AssetScale, 1, 1000);

    this->_editorsByParam[AtomParameter::PenWidth] = new AtomSliderEditor(AtomParameter::PenWidth, 1, 50);
    this->_editorsByParam[AtomParameter::TextSize] = new AtomSliderEditor(AtomParameter::TextSize, 1, 50);
    this->_editorsByParam[AtomParameter::Text] = new AtomTextEditor(AtomParameter::Text);

    for(auto editor : this->_editorsByParam) {

        QObject::connect(
            editor, &AtomSubEditor::valueConfirmedForPayload,
            this, &AtomEditor::_onSubEditorChanged
        );

        QObject::connect(
            editor, &AtomSubEditor::valueConfirmedForPreview,
            this, &AtomEditor::_onPreviewRequested
        );

        this->layout()->addWidget(editor);

    }
}

void AtomEditor::_onSubEditorChanged(const AtomParameter &parameterWhoChanged, QVariant &value) {
    
    if(this->_currentEditMode == Template) {
        AtomTemplateChangedPayload payload(parameterWhoChanged, value);
        this->_emitPayload(payload);
    } 
    
    else {
        MetadataChangedPayload payload(this->_RPZAtomIds, parameterWhoChanged, value);
        this->_emitPayload(payload);
    }

}

void AtomEditor::_onPreviewRequested(const AtomParameter &parameter, QVariant &value) {
    emit requiresPreview(this->_RPZAtomIds, parameter, value);
}

void AtomEditor::_emitPayload(AlterationPayload &payload) {
    AlterationHandler::get()->queueAlteration(AlterationPayload::Source::Local_AtomEditor, payload);
}

AtomUpdates AtomEditor::_findDefaultValuesToBind() {
    AtomUpdates out;

    if(this->_atoms.count() > 0) {

        auto firstItem = this->_atoms[0];

        //intersect all customizables params
        QSet<AtomParameter> paramsToDisplay = firstItem->customizableParams();
        for(int i = 1; i < this->_atoms.count(); i++) {
            auto currCP = this->_atoms[i]->customizableParams();
            paramsToDisplay = paramsToDisplay.intersect(currCP);
        }

        //find default values to apply
        auto hasMoreThanOneAtomToBind = this->_atoms.count() > 1;
        for(auto param : paramsToDisplay) {
            QVariant val =  hasMoreThanOneAtomToBind ? QVariant() : firstItem->metadata(param);
            out.insert(param, val);
        }

    }

    return out;
}


void AtomEditor::_updateEditMode() {
    auto countItems = this->_atoms.count();
    auto firstItem = countItems > 0 ? this->_atoms.at(0) : nullptr;
    
    if(countItems == 0) {
        this->_currentEditMode  = EditMode::None;
    }
    else if(countItems == 1 && !firstItem->id()) {
        this->_currentEditMode = EditMode::Template;
    } 
    else {
        this->_currentEditMode = EditMode::Selection;
    }

    //update title
    this->setTitle(_strEM[this->_currentEditMode]);
}