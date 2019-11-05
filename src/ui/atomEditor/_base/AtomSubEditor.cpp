#include "AtomSubEditor.h"

AtomSubEditor::AtomSubEditor(const QList<RPZAtom::Parameter> &parameters, bool supportsBatchEditing) :
    _supportsBatchEditing(supportsBatchEditing),
    _descr(new AtomEditorLineDescriptor(parameters.first(), supportsBatchEditing)),
    _params(parameters) { 

    this->setVisible(false);

    this->setLayout(new QVBoxLayout);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    this->layout()->addWidget(this->_descr);

}

const QList<RPZAtom::Parameter> AtomSubEditor::params() {
    return this->_params;
}

void AtomSubEditor::_setAsDataEditor(QWidget *dataEditor) {
    this->_dataEditor = dataEditor;
    this->layout()->addWidget(dataEditor);
}

bool AtomSubEditor::mustShowBrushPenWidth(const QVariant &brushTypeDefaultValue) {
    auto cast = (RPZAtom::BrushType)brushTypeDefaultValue.toInt();
    return cast == RPZAtom::BrushType::RoundBrush;
}


void AtomSubEditor::_handleVisibilityOnLoad(const RPZAtom::Updates &defaultValues) {
    
    //default behavior if not a penWidth param
    if(!this->_params.contains(RPZAtom::Parameter::BrushPenWidth)) return this->setVisible(true);

    //check
    auto brushStyleVal = defaultValues.value(RPZAtom::Parameter::BrushStyle);
    if(brushStyleVal.isNull()) return;

    //visibility
    auto mustShow = mustShowBrushPenWidth(brushStyleVal);
    this->setVisible(mustShow);
}


void AtomSubEditor::loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::EditMode &editMode) {
    
    //handle visibility
    this->_handleVisibilityOnLoad(defaultValues);

    auto hasEmptyValue = false;
    for(auto &param : this->_params) {
        auto associatedValue = defaultValues.value(param);
        if(associatedValue.isNull()) hasEmptyValue = true;
    }
    
    //replace descr if has empty
    if(hasEmptyValue) this->_descr->cannotDisplayValue();

}
