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

void AtomSubEditor::loadTemplate(const QVariant &defaultValue) {
    
    this->setVisible(true);
    
    if(defaultValue.isNull()) {
        this->_descr->cannotDisplayValue();
    }

}
