#include "AtomSliderEditor.h"

AtomSliderEditor::AtomSliderEditor(const AtomParameter &parameter, int minimum, int maximum) : AtomSubEditor(parameter) { 

    this->_setAsDataEditor(new RPZCustomSlider(Qt::Orientation::Horizontal, this));
    this->slider()->setMinimum(minimum);
    this->slider()->setMaximum(maximum);

    QObject::connect(
        this->slider(), &QAbstractSlider::sliderReleased,
        [&]() {
            auto out = QVariant(this->outputValue());
            emit valueConfirmedForPayload(this->_param, out);
        }
    );

    QObject::connect(
        this->slider(), &QAbstractSlider::valueChanged,
        this, &AtomSliderEditor::_onSliderChanging
    );

}

void AtomSliderEditor::_onSliderChanging(int sliderVal) {
    
    auto output = this->outputValue();
    auto outputAsVariant = QVariant(output);
    this->_descr->updateValue(output);
    
    emit valueConfirmedForPreview(this->_param, outputAsVariant);
    
};


QSlider* AtomSliderEditor::slider() {
    return (QSlider*)this->_dataEditor;
}


void AtomSliderEditor::loadTemplate(const QVariant &defaultValue, bool updateMode) {
    
    AtomSubEditor::loadTemplate(defaultValue, updateMode);

    auto castedVal = defaultValue.toDouble();

    if(!defaultValue.isNull()) {
        this->_descr->updateValue(castedVal);
    }
    
    {
        QSignalBlocker b(this->slider());
        auto sval = this->_toSliderValue(castedVal);
        this->slider()->setValue(sval);
    }

}


double AtomSliderEditor::outputValue() {
    return this->_toAtomValue(this->slider()->value());
}

double AtomSliderEditor::_toAtomValue(int sliderVal) {
    return sliderVal;
}

int AtomSliderEditor::_toSliderValue(double atomValue) {
    return (int)atomValue;
}