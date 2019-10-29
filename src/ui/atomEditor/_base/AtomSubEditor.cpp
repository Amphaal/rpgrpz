#include "AtomSubEditor.h"

AtomSubEditor::AtomSubEditor(const RPZAtom::Parameter &parameter) :
    _descr(new AtomEditorLineDescriptor(parameter)),
    _param(parameter) { 

    this->setVisible(false);

    this->setLayout(new QVBoxLayout);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    this->layout()->addWidget(this->_descr);

}

RPZAtom::Parameter AtomSubEditor::param() {
    return this->_param;
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
    if(this->_param != RPZAtom::Parameter::BrushPenWidth) return this->setVisible(true);

    //check
    auto brushStyleVal = defaultValues.value(RPZAtom::Parameter::BrushStyle);
    if(brushStyleVal.isNull()) return;

    //visibility
    auto mustShow = mustShowBrushPenWidth(brushStyleVal);
    this->setVisible(mustShow);
}


QVariant AtomSubEditor::loadTemplate(const RPZAtom::Updates &defaultValues, bool updateMode) {
    
    //handle visibility
    this->_handleVisibilityOnLoad(defaultValues);
    
    //replace descr if empty
    auto defaultValue = defaultValues.value(this->_param);
    if(defaultValue.isNull()) {
        this->_descr->cannotDisplayValue();
    }

    return defaultValue;

}
