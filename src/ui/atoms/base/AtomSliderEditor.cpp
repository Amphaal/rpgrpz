#include "AtomSliderEditor.h"

AtomSliderEditor::AtomSliderEditor(const QString &descr, const QString &suffix, int minimum, int maximum) :
    _descr(new AtomEditorLineDescriptor(descr, suffix)),
    _slider(new QSlider(Qt::Orientation::Horizontal, this)) { 

    this->setVisible(false);

    this->setLayout(new QVBoxLayout);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    this->_slider->setMinimum(minimum);
    this->_slider->setMaximum(maximum);

    this->layout()->addWidget(this->_descr);
    this->layout()->addWidget(this->_slider);

    QObject::connect(
        this->_slider, &QAbstractSlider::valueChanged,
        this, &AtomSliderEditor::_onSliderChanging
    );
}

QSlider* AtomSliderEditor::slider() {
    return this->_slider;
}

void AtomSliderEditor::loadAtomsAsTemplate(QVector<RPZAtom*> &atoms) {
    
    this->_atoms = atoms;
    if(!this->_atoms.count()) return this->setVisible(false);
    
    this->setVisible(true);
    auto atomTemplate = this->atomTemplate();
    auto multipleAtoms = this->_atoms.count() > 1;
    
    if(multipleAtoms) {
        this->_descr->cannotDisplayValue();
    } else {
        auto val = this->atomValue(atomTemplate);
        this->_descr->updateValue(val);
    }
    
    this->_slider->blockSignals(true); 
        auto sval = this->atomValueToSliderValue(atomTemplate);
        this->_slider->setValue(sval);
    this->_slider->blockSignals(false);

}

void AtomSliderEditor::_onSliderChanging(int sliderVal) {
    
    auto output = this->outputValue();
    this->_descr->updateValue(output);
    
    for(auto atom : this->_atoms) {
        this->_updateGraphicsItem(atom->graphicsItem(), output);
    }
};

double AtomSliderEditor::outputValue() {
    return this->_slider->value();
}

RPZAtom AtomSliderEditor::atomTemplate() {
    return this->_atoms.count() == 1 ? *this->_atoms[0] : RPZAtom();
}

QVector<snowflake_uid> AtomSliderEditor::_atomsToSnowflakeList() {
    QVector<snowflake_uid> out;
    for(auto atom : this->_atoms) out.append(atom->id());
    return out;
}
