#include "AtomEditor.h"

AtomEditor::AtomEditor(QWidget* parent) : QWidget(parent) {

    //prepare
    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    
    auto layout = new QVBoxLayout;
    this->setLayout(layout);

    //editors
    this->_rotateEditor = new AtomRotationEditor;
    this->_scaleEditor = new AtomScalingEditor;
    this->_penWidthEditor = new AtomPenWidthEditor;
    
    layout->addWidget(this->_rotateEditor);
    layout->addWidget(this->_scaleEditor);
    layout->addWidget(this->_penWidthEditor);

    QObject::connect(
        this->_rotateEditor->slider(), &QAbstractSlider::sliderReleased,
        [=]() { this->_onSubEditorChanged(this->_rotateEditor); }
    );
    QObject::connect(
        this->_scaleEditor->slider(), &QAbstractSlider::sliderReleased,
        [=]() { this->_onSubEditorChanged(this->_scaleEditor); }
    );
    QObject::connect(
        this->_penWidthEditor->slider(), &QAbstractSlider::sliderReleased,
        [=]() { this->_onSubEditorChanged(this->_penWidthEditor); }
    );
}


void AtomEditor::_onSubEditorChanged(AtomSliderEditor* editor) {
    auto payload = editor->createPayload();
    requiresAtomAlteration(payload);
}

void AtomEditor::buildEditorFromSelection(QVector<void*> &selectedAtoms) {
    
    //populate
    this->_atoms.clear();
    for(auto atom : selectedAtoms) this->_atoms.append((RPZAtom*)atom);

    this->_rotateEditor->loadAtomsAsTemplate(this->_atoms);
    this->_scaleEditor->loadAtomsAsTemplate(this->_atoms);
    this->_penWidthEditor->loadAtomsAsTemplate(this->_atoms);
}