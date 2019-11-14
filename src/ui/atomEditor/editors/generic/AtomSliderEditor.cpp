#include "AtomSliderEditor.h"

AtomSliderEditor::AtomSliderEditor(const RPZAtom::Parameter &parameter, int minimum, int maximum) : AtomSubEditor({parameter}) { 

    this->_commitTimer.setInterval(200);
    this->_commitTimer.setSingleShot(true);

    this->_slider = new QSlider(Qt::Orientation::Horizontal, this);
    this->_slider->setMinimum(minimum);
    this->_slider->setMaximum(maximum);
    this->layout()->addWidget(this->_slider);

    QObject::connect(
        this->_slider, &QAbstractSlider::valueChanged,
        this, &AtomSliderEditor::_onValueChanged
    );

    QObject::connect(
        &this->_commitTimer, &QTimer::timeout,
        this, &AtomSliderEditor::_confirmPayload
    );

}

void AtomSliderEditor::_onValueChanged(int sliderVal) {
    this->_confirmPreview();
    this->_commitTimer.start();  
};

void AtomSliderEditor::_confirmPayload() {
    auto out = QVariant(this->outputValue());
    emit valueConfirmedForPayload({{this->_params.first(), out}});
}

void AtomSliderEditor::_confirmPreview() {
    
    auto output = this->outputValue();
    this->_descr->updateValue(output);
    
    emit valueConfirmedForPreview(this->_params.first(), output);

}

void AtomSliderEditor::loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::EditMode &editMode) {
    
    AtomSubEditor::loadTemplate(defaultValues, editMode);
    
    bool success = false;
    auto defaultVal = defaultValues[this->_params.first()].toDouble(&success);

    if(success) {
        this->_descr->updateValue(defaultVal);
    }
      
    QSignalBlocker b(this->_slider);
    auto sval = this->_toSliderValue(defaultVal);
    this->_slider->setValue(sval);

}

double AtomSliderEditor::outputValue() {
    return this->_toAtomValue(this->_slider->value());
}

double AtomSliderEditor::_toAtomValue(int sliderVal) {
    return sliderVal;
}

int AtomSliderEditor::_toSliderValue(double atomValue) {
    return (int)atomValue;
}