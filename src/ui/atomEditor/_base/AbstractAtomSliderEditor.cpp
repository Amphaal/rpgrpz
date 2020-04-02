// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

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
    this->_slider->setMinimum(this->_ceData.minSlider());
    this->_slider->setMaximum(this->_ceData.maxSlider());
    QObject::connect(
        this->_slider, &QAbstractSlider::valueChanged,
        this, &AbstractAtomSliderEditor::_onSliderValueChanged
    );

    this->_widgetLineLayout = new QHBoxLayout;
    this->_widgetLineLayout->addWidget(this->_slider, 1);

    this->_mainLayout->addLayout(this->_widgetLineLayout);

}

void AbstractAtomSliderEditor::loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {
    
    AtomSubEditor::loadTemplate(defaultValues, context);
    
    bool success = false;
    auto defaultVal = defaultValues[this->_params.first()].toDouble(&success);

    if(success) {
        this->_descr->updateValue(defaultVal);
    }
    
    this->_initialSetup(defaultVal);

}

const CrossEquities& AbstractAtomSliderEditor::_crossEquities() const {
    return this->_ceData;
}

void AbstractAtomSliderEditor::_onSliderValueChanged(int sliderVal) {
    auto atomVal = this->toAtomValue(sliderVal);
    this->_updateSpinner(atomVal);
    this->_triggerAlterations();
};

void AbstractAtomSliderEditor::_triggerAlterations() {
    this->_confirmPreview();
    this->_commitTimer.start();  
}

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

void AbstractAtomSliderEditor::_initialSetup(double atomVal) {
    auto sval = this->toSliderValue(atomVal);
    this->_updateSlider(sval);
    this->_updateSpinner(atomVal);
}

void AbstractAtomSliderEditor::_updateSlider(int toApply) {
    QSignalBlocker l(this->_slider);
    this->_slider->setValue(toApply);
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