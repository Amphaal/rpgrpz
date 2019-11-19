#include "AtomSubEditor.h"

AtomSubEditor::AtomSubEditor(const QList<RPZAtom::Parameter> &parameters, bool supportsBatchEditing) :
    _supportsBatchEditing(supportsBatchEditing),
    _descr(new AtomEditorLineDescriptor(parameters.first(), supportsBatchEditing)),
    _params(parameters) { 

    this->setVisible(false);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    this->_mainLayout = new QVBoxLayout;
    this->setLayout(this->_mainLayout);
    this->layout()->addWidget(this->_descr);

}

const QList<RPZAtom::Parameter> AtomSubEditor::params() {
    return this->_params;
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


void AtomSubEditor::loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {
    
    this->setEnabled(true);

    //handle visibility
    this->_handleVisibilityOnLoad(defaultValues);

    auto hasEmptyValue = false;
    for(const auto &param : this->_params) {
        auto associatedValue = defaultValues.value(param);
        if(associatedValue.isNull()) hasEmptyValue = true;
    }
    
    //replace descr if has empty
    if(hasEmptyValue) {
        this->_descr->cannotDisplayValue();
        if(!this->_supportsBatchEditing) this->setEnabled(false);
    }

}
