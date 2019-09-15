#include "AtomEditor.h"

AtomEditor::AtomEditor(QWidget* parent) : QGroupBox(_strEM[None], parent), AlterationActor(AlterationPayload::Source::Local_AtomEditor) {

    this->setAlignment(Qt::AlignHCenter);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    this->setLayout(new QVBoxLayout);
    
    //create params editors
    this->_createEditorsFromAtomParameters();

    //message indicating no editors available
    this->_noEditorMsgWidget = new NoEditorMessageWidget;
    this->layout()->addWidget(this->_noEditorMsgWidget);

}

void AtomEditor::buildEditor(const AtomsSelectionDescriptor &atomsSelectionDescr) {
    
    //modify atom list
    this->_currentSelectionDescr = atomsSelectionDescr;
    
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
        editor->loadTemplate(i.value());

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

    //if no editor is displayed, show a little message
    this->_noEditorMsgWidget->setVisible(!this->hasVisibleEditors());

}

//
//
//

void AtomEditor::resetParams() {

    //reset displayed params
    AtomUpdates changes;
    for(auto param : this->_visibleEditors) {
        changes.insert(
            param, 
            RPZAtom::getDefaultValueForParam(param)
        );
    }

    //update inner selection accordingly
    this->_currentSelectionDescr.templateAtom.setMetadata(changes);

    //rebuild
    this->buildEditor(this->_currentSelectionDescr);

    //emit modifications
    this->_emitPayload(changes);

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
            this, &AtomEditor::_emitPayloadCB
        );

        QObject::connect(
            editor, &AtomSubEditor::valueConfirmedForPreview,
            this, &AtomEditor::_onPreviewRequested
        );

        this->layout()->addWidget(editor);

    }
}

void AtomEditor::_onPreviewRequested(const AtomParameter &parameter, const QVariant &value) {
    emit requiresPreview(this->_currentSelectionDescr, parameter, value);
}

 void AtomEditor::_emitPayloadCB(const AtomParameter &parameter, const QVariant &value) {
    return _emitPayload({{parameter, value}});
 }

void AtomEditor::_emitPayload(const AtomUpdates &changesToEmit) {

    if(this->_currentEditMode == Template) {
        AtomTemplateChangedPayload payload(changesToEmit);
        AlterationHandler::get()->queueAlteration(this, payload);
    } 
    
    else {

        MetadataChangedPayload payload(
            this->_currentSelectionDescr.selectedAtomIds, 
            changesToEmit
        );

        AlterationHandler::get()->queueAlteration(this, payload);
    }

}

AtomUpdates AtomEditor::_findDefaultValuesToBind() {
    
    AtomUpdates out;

    //intersect represented atom types in selection to determine which editors to display
    QSet<AtomParameter> paramsToDisplay;
    for(auto &type : this->_currentSelectionDescr.representedTypes) {
        
        auto associatedCustomParams = RPZAtom::customizableParams(type);
        
        if(paramsToDisplay.empty()) {
            paramsToDisplay = associatedCustomParams;
            continue;
        }

        paramsToDisplay = paramsToDisplay.intersect(associatedCustomParams);

    }

    for(auto param : paramsToDisplay) {
        out.insert(
            param, 
            this->_currentSelectionDescr.templateAtom.metadata(param)
        );
    }

    return out;
}


void AtomEditor::_updateEditMode() {
    
    //determine edit mode
    auto selectedIdsCount = this->_currentSelectionDescr.selectedAtomIds.count();
    
    if(selectedIdsCount) {
        this->_currentEditMode = EditMode::Selection;
    }
    else if(!selectedIdsCount && !this->_currentSelectionDescr.templateAtom.isEmpty()) {
        this->_currentEditMode = EditMode::Template;
    } 
    else {
        this->_currentEditMode  = EditMode::None;
    }

    //update title
    auto title = _strEM[this->_currentEditMode];
    
    switch(this->_currentEditMode) {
        
        case EditMode::Selection:
            title += QString(" (%1 élement%2)").arg(selectedIdsCount).arg(selectedIdsCount > 1 ? "s" : "");
        break;

        case EditMode::Template:
            title += QString(" (%1)").arg(this->_currentSelectionDescr.templateAtom.assetName());
        break;

        default:
        break;

    }

    this->setTitle(title);
}

bool AtomEditor::hasVisibleEditors() {
    return this->_visibleEditors.count();
}

AtomsSelectionDescriptor AtomEditor::currentSelectionDescriptor() {
    return this->_currentSelectionDescr;
}