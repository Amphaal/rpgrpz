#include "AtomSubEditor.h"

AtomSubEditor::AtomSubEditor(const AtomParameter &parameter) :
    _descr(new AtomEditorLineDescriptor(parameter)),
    _param(parameter) { 

    this->setVisible(false);

    this->setLayout(new QVBoxLayout);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    this->layout()->addWidget(this->_descr);

}

AtomParameter AtomSubEditor::param() {
    return this->_param;
}

void AtomSubEditor::_setAsDataEditor(QWidget *dataEditor) {
    this->_dataEditor = dataEditor;
    this->layout()->addWidget(dataEditor);
}

bool AtomSubEditor::mustShowBrushPenWidth(const QVariant &brushTypeDefaultValue) {
    auto cast = (BrushType)brushTypeDefaultValue.toInt();
    return cast == BrushType::RoundBrush;
}


void AtomSubEditor::_handleVisibilityOnLoad(const AtomUpdates &defaultValues) {
    
    //default behavior if not a penWidth param
    if(this->_param != AtomParameter::BrushPenWidth) return this->setVisible(true);

    //check
    auto brushStyleVal = defaultValues.value(AtomParameter::BrushStyle);
    if(brushStyleVal.isNull()) return;

    //visibility
    auto mustShow = mustShowBrushPenWidth(brushStyleVal);
    this->setVisible(mustShow);
}


QVariant AtomSubEditor::loadTemplate(const AtomUpdates &defaultValues, bool updateMode) {
    
    //handle visibility
    this->_handleVisibilityOnLoad(defaultValues);
    
    //replace descr if empty
    auto defaultValue = defaultValues.value(this->_param);
    if(defaultValue.isNull()) {
        this->_descr->cannotDisplayValue();
    }

    return defaultValue;

}
