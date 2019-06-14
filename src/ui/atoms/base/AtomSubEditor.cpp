#include "AtomSubEditor.h"

AtomSubEditor::AtomSubEditor(const RPZAtom::Parameters &parameter) :
    _param(parameter),
    _descr(new AtomEditorLineDescriptor(_ParamDescr[parameter], _valSuffix[parameter])),
    _dataEditor(this->_instDataEditor()) { 

    this->setVisible(false);

    this->setLayout(new QVBoxLayout);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    this->layout()->addWidget(this->_descr);
    this->layout()->addWidget(this->_dataEditor);

}

void AtomSubEditor::loadTemplate(QVector<RPZAtom*> &atomsToManipulate, QVariant &defaultValue) {
    
    this->_atomsToManipulate = atomsToManipulate;
    if(!this->_atomsToManipulate.count()) return this->setVisible(false);
    
    this->setVisible(true);
    
    if(defaultValue.isNull()) {
        this->_descr->cannotDisplayValue();
    }

}