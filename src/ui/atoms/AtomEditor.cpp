#include "AtomEditor.h"

AtomEditor::AtomEditor(QWidget* parent) : QWidget(parent) {
    
    //prepare
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    this->setLayout(new QVBoxLayout);

    //editors
    this->_rotateEditor = new AtomRotationEditor(this);
    this->_scaleEditor = new AtomScalingEditor(this);
}

void AtomEditor::signalPayload(AlterationPayload &payload) {
    emit requiresAtomAlteration(payload);
}

void AtomEditor::buildEditorFromSelection(QVector<void*> &selectedAtoms) {
    
    //populate
    this->_atoms.clear();
    for(auto atom : selectedAtoms) this->_atoms.append((RPZAtom*)atom);

    //if no selection
    if(!selectedAtoms.count()) {
        this->_destroyEditor();
        return;
    } 
    
    //if no items
    if(!this->layout()->count()) {
        this->_rotateEditor->loadAtomsAsTemplate(this->_atoms);
        this->_scaleEditor->loadAtomsAsTemplate(this->_atoms);

        this->layout()->addWidget(this->_rotateEditor);
        this->layout()->addWidget(this->_scaleEditor);
    }

}

void AtomEditor::_destroyEditor() {
    while (auto item = this->layout()->takeAt(0)) {
        this->layout()->removeItem(item);
    }
}
