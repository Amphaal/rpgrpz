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

void AtomSubEditor::loadTemplate(QVector<RPZAtom*> &atomsToManipulate, QVariant &defaultValue) {
    
    this->_atomsToManipulate = atomsToManipulate;
    if(!this->_atomsToManipulate.count()) return this->setVisible(false);
    
    this->setVisible(true);
    
    if(defaultValue.isNull()) {
        this->_descr->cannotDisplayValue();
    }

}
