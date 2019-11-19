#include "AbstractAtomSliderEditor.h"

AbstractAtomSliderEditor::AbstractAtomSliderEditor(const RPZAtom::Parameter &parameter, const CrossEquities &crossEquities) : AtomSubEditor({parameter}),
    _ceData(crossEquities) { 

    //debouncer
    this->_commitTimer.setInterval(200);
    this->_commitTimer.setSingleShot(true);
    QObject::connect(
        &this->_commitTimer, &QTimer::timeout,
        this, &AbstractAtomSliderEditor::_confirmPayload
    );

    //define slider
    this->_slider = new QSlider(Qt::Orientation::Horizontal, this);
    this->_slider->setMinimum(this->_ceData.v().first().sliderValue);
    this->_slider->setMaximum(this->_ceData.v().last().sliderValue);
    QObject::connect(
        this->_slider, &QAbstractSlider::valueChanged,
        this, &AbstractAtomSliderEditor::_onSliderValueChanged
    );

    //generate spin
    this->_spin = this->_generateSpinBox();

    auto widgetLineLayout = new QHBoxLayout;
    widgetLineLayout->addWidget(this->_slider, 1);
    widgetLineLayout->addWidget(this->_spin);

    this->_mainLayout->addLayout(widgetLineLayout);

}

void AbstractAtomSliderEditor::loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {
    
    AtomSubEditor::loadTemplate(defaultValues, context);
    
    bool success = false;
    auto defaultVal = defaultValues[this->_params.first()].toDouble(&success);

    if(success) {
        this->_descr->updateValue(defaultVal);
    }
    
    this->_updateWidgetsFromAtomVal(defaultVal);

}

const CrossEquities& AbstractAtomSliderEditor::_crossEquities() const {
    return this->_crossEquities;
}

void AbstractAtomSliderEditor::_onSliderValueChanged(int sliderVal) {
    this->_updateWidgetsFromSliderVal(sliderVal);
    this->_confirmPreview();
    this->_commitTimer.start();  
};

void AbstractAtomSliderEditor::_confirmPayload() {
    auto output = this->outputValue();
    auto param = this->_params.first();
    emit valueConfirmedForPayload({{param, output}});
}

void AbstractAtomSliderEditor::_confirmPreview() {
    
    auto output = this->outputValue();
    auto param = this->_params.first();

    this->_descr->updateValue(output);

    emit valueConfirmedForPreview(param, output);

}

void AbstractAtomSliderEditor::_updateWidgetsFromAtomVal(double atomVal) {
    auto sval = this->toSliderValue(atomVal);
    this->_updateWidgetsFromSliderVal(sval);
}

void AbstractAtomSliderEditor::_updateWidgetsFromSliderVal(int sliderVal) {
    QSignalBlocker l(this->_slider);
    this->_slider->setValue(sliderVal);
}

double AbstractAtomSliderEditor::toAtomValue(int sliderVal) const {
    return this->_ceData.toAtomValue(sliderVal);
}
int AbstractAtomSliderEditor::toSliderValue(double atomVal) const {
    return this->_ceData.toSliderValue(atomVal);
}

double AbstractAtomSliderEditor::outputValue() const {
    return this->toAtomValue(this->_slider->value());
}