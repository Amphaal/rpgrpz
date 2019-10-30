#include "AtomEditor.h"

AtomEditor::AtomEditor(QWidget* parent) : QGroupBox(parent), AlterationActor(Payload::Source::Local_AtomEditor) {

    auto title = _strEM.value(EditMode::None);
    this->setTitle(tr(qUtf8Printable(title)));
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
    auto isUpdateMode = this->_currentEditMode == EditMode::Selection;

    //load those who need to be displayed
    for(auto i = toDisplay.begin(); i != toDisplay.end(); ++i) {
        
        //prepare
        auto param = i.key();

        //get editor
        auto editor = this->_editorsByParam.value(param);
        if(!editor) continue;

        //load template, and display them
        editor->loadTemplate(toDisplay, isUpdateMode);

        //add to the visible editors list
        this->_visibleEditors.append(param);

    }

    //hide the others
    auto toHide = _editorsByParam.keys().toSet().subtract(
        toDisplay.keys().toSet()
    );
    for(auto i : toHide) {
        auto editor = this->_editorsByParam.value(i);
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
    RPZAtom::Updates changes;
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

    this->_editorsByParam.insert(RPZAtom::Parameter::BrushStyle, new BrushToolEditor);
    this->_editorsByParam.insert(RPZAtom::Parameter::BrushPenWidth, new AtomSliderEditor(RPZAtom::Parameter::BrushPenWidth, 1, 500));

    this->_editorsByParam.insert(RPZAtom::Parameter::Rotation, new AtomSliderEditor(RPZAtom::Parameter::Rotation, 0, 359));
    this->_editorsByParam.insert(RPZAtom::Parameter::Scale, new NonLinearAtomSliderEditor(RPZAtom::Parameter::Scale, 1, 1000));
    
    this->_editorsByParam.insert(RPZAtom::Parameter::AssetRotation, new AtomSliderEditor(RPZAtom::Parameter::AssetRotation, 0, 359));
    this->_editorsByParam.insert(RPZAtom::Parameter::AssetScale, new NonLinearAtomSliderEditor(RPZAtom::Parameter::AssetScale, 1, 1000));

    this->_editorsByParam.insert(RPZAtom::Parameter::PenWidth, new AtomSliderEditor(RPZAtom::Parameter::PenWidth, 1, 50));
    this->_editorsByParam.insert(RPZAtom::Parameter::TextSize, new AtomSliderEditor(RPZAtom::Parameter::TextSize, 1, 50));
    this->_editorsByParam.insert(RPZAtom::Parameter::Text, new AtomTextEditor(RPZAtom::Parameter::Text));

    this->_editorsByParam.insert(RPZAtom::Parameter::ShortDescription, new AtomShortTextEditor(RPZAtom::Parameter::ShortDescription));
    this->_editorsByParam.insert(RPZAtom::Parameter::Description, new AtomTextEditor(RPZAtom::Parameter::Description));


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

void AtomEditor::_onPreviewRequested(const RPZAtom::Parameter &parameter, const QVariant &value) {
    emit requiresPreview(this->_currentSelectionDescr, parameter, value);
}

 void AtomEditor::_emitPayloadCB(const RPZAtom::Parameter &parameter, const QVariant &value) {
    
    //intercept combo change for visibility
    this->_mustShowBrushPenWidthEditor(parameter, value);
    
    return _emitPayload({{parameter, value}});
 }

void AtomEditor::_emitPayload(const RPZAtom::Updates &changesToEmit) {

    if(this->_currentEditMode == EditMode::Template) {
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

RPZAtom::Updates AtomEditor::_findDefaultValuesToBind() {
    
    RPZAtom::Updates out;

    //intersect represented atom types in selection to determine which editors to display
    QSet<RPZAtom::Parameter> paramsToDisplay;
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
        this->_currentEditMode = EditMode::None;
    }

    //update title
    auto title = tr(qUtf8Printable(_strEM.value(this->_currentEditMode)));
    
    switch(this->_currentEditMode) {
        
        case EditMode::Selection:
            title += tr(" (%n element(s))", "", selectedIdsCount);
        break;

        case EditMode::Template:
            title += QStringLiteral(u" [%1]").arg(this->_currentSelectionDescr.templateAtom.toString());
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

void AtomEditor::_mustShowBrushPenWidthEditor(const RPZAtom::Parameter &paramToCheck, const QVariant &defaultValue) {

    //check if param is tool combo
    if(paramToCheck != RPZAtom::Parameter::BrushStyle) return;

    //check if pen size editor exists
    auto brushPenWidthEditor = this->_editorsByParam.value(RPZAtom::Parameter::BrushPenWidth);
    if(!brushPenWidthEditor) return;

    //set visibility
    auto mustShow = AtomSubEditor::mustShowBrushPenWidth(defaultValue);
    brushPenWidthEditor->setVisible(mustShow);

}