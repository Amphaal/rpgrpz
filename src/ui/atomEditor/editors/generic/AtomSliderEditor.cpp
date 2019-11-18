#include "AtomSliderEditor.h"

AtomSliderEditor::AtomSliderEditor(const RPZAtom::Parameter &parameter, int minimum, int maximum) : AtomSubEditor({parameter}) { 

    this->_commitTimer.setInterval(200);
    this->_commitTimer.setSingleShot(true);

    this->_spin = new QSpinBox(this);
    this->_spin->setMinimum(minimum);
    this->_spin->setMaximum(maximum);

    this->_slider = new QSlider(Qt::Orientation::Horizontal, this);
    this->_slider->setMinimum(minimum);
    this->_slider->setMaximum(maximum);

    auto sl = new QHBoxLayout;
    sl->addWidget(this->_slider, 1);
    sl->addWidget(this->_spin, 0);
    
    auto l = (QVBoxLayout*)this->layout();
    l->addLayout(sl);

    QObject::connect(
        this->_slider, &QAbstractSlider::valueChanged,
        this, &AtomSliderEditor::_onValueChanged
    );

    QObject::connect(
        this->_slider, &QAbstractSlider::valueChanged,
        this, &AtomSliderEditor::_onValueChanged
    );

    QObject::connect(
        this->_spin, qOverload<int>(&QSpinBox ::valueChanged),
        this, &AtomSliderEditor::_confirmPayload
    );

}

void AtomSliderEditor::_onValueChanged(int sliderVal) {
    this->_internalWidgetsUpdate(sliderVal);
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

void AtomSliderEditor::loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {
    
    AtomSubEditor::loadTemplate(defaultValues, context);
    
    bool success = false;
    auto defaultVal = defaultValues[this->_params.first()].toDouble(&success);

    if(success) {
        this->_descr->updateValue(defaultVal);
    }
      
    auto sval = this->_toSliderValue(defaultVal);
    this->_internalWidgetsUpdate(sval);

}

void AtomSliderEditor::_internalWidgetsUpdate(int sliderVal) {

    QSignalBlocker l(this->_slider);
    QSignalBlocker l2(this->_spin);

    this->_spin->setValue(sliderVal);
    this->_slider->setValue(sliderVal);

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