#include "AtomSliderEditor.h"

AtomSliderEditor::AtomSliderEditor(const RPZAtom::Parameters &parameter, int minimum, int maximum) : AtomSubEditor(parameter) { 

    this->_setAsDataEditor(new QSlider(Qt::Orientation::Horizontal, this));
    this->slider()->setMinimum(minimum);
    this->slider()->setMaximum(maximum);

    QObject::connect(
        this->slider(), &QAbstractSlider::valueChanged,
        this, &AtomSliderEditor::_onSliderChanging
    );

    QObject::connect(
        this->slider(), &QAbstractSlider::sliderReleased,
        [&]() {
            emit valueConfirmedForPayload(this->_param, QVariant(this->outputValue()));
        }
    );

}

QSlider* AtomSliderEditor::slider() {
    return (QSlider*)this->_dataEditor;
}


void AtomSliderEditor::loadTemplate(QVector<RPZAtom*> &atomsToManipulate, QVariant &defaultValue) {
    
    AtomSubEditor::loadTemplate(atomsToManipulate, defaultValue);

    auto castedVal = defaultValue.toDouble();

    if(!defaultValue.isNull()) {
        this->_descr->updateValue(castedVal);
    }
    
    this->slider()->blockSignals(true); 
        auto sval = this->_toSliderValue(castedVal);
        this->slider()->setValue(sval);
    this->slider()->blockSignals(false);

}

void AtomSliderEditor::_onSliderChanging(int sliderVal) {
    
    auto output = this->outputValue();
    this->_descr->updateValue(output);
    
    for(auto atom : this->_atomsToManipulate) {
        RPZAtom::updateGraphicsItemFromMetadata(atom->graphicsItem(), this->_param, QVariant(output));
    }
};

double AtomSliderEditor::outputValue() {
    return this->_toAtomValue(this->slider()->value());
}

double AtomSliderEditor::_toAtomValue(int sliderVal) {
    return sliderVal;
}

int AtomSliderEditor::_toSliderValue(double atomValue) {
    return atomValue;
}