#include "AtomSliderEditor.h"

AtomSliderEditor::AtomSliderEditor(const RPZAtom::Parameter &parameter, int minimum, int maximum) : AtomSubEditor({parameter}) { 

    this->_setAsDataEditor(new RPZCustomSlider(Qt::Orientation::Horizontal, this));
    this->slider()->setMinimum(minimum);
    this->slider()->setMaximum(maximum);

    QObject::connect(
        this->slider(), &QAbstractSlider::sliderReleased,
        [&]() {
            auto out = QVariant(this->outputValue());
            emit valueConfirmedForPayload({{this->_params.first(), out}});
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
    
    emit valueConfirmedForPreview(this->_params.first(), outputAsVariant);
    
};


QSlider* AtomSliderEditor::slider() {
    return (QSlider*)this->_dataEditor;
}

const AtomSubEditor::FilteredDefaultValues AtomSliderEditor::loadTemplate(const RPZAtom::Updates &defaultValues, bool updateMode) {
    
    auto filtered = AtomSubEditor::loadTemplate(defaultValues, updateMode);
    
    auto defaultVal = filtered[this->_params.first()];
    auto castedVal = filtered[this->_params.first()].toDouble();

    if(!defaultVal.isNull()) {
        this->_descr->updateValue(castedVal);
    }
      
    QSignalBlocker b(this->slider());
    auto sval = this->_toSliderValue(castedVal);
    this->slider()->setValue(sval);
    
    return filtered;

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