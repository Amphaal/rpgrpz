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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include "src/ui/atomEditor/_base/AbstractAtomSliderEditor.h"

#include <QtMath>
#include <QDoubleSpinBox>

class NonLinearAtomSliderEditor : public AbstractAtomSliderEditor {
 public:
        NonLinearAtomSliderEditor(const RPZAtom::Parameter &parameter, const CrossEquities &crossEquities) : AbstractAtomSliderEditor(parameter, crossEquities) {
            this->_spin = this->_generateSpinBox();
            this->_widgetLineLayout->addWidget(this->_spin);
        }

 protected: 
        QAbstractSpinBox* _generateSpinBox() const override {
            
            auto spin = new QDoubleSpinBox;
 
            spin->setMinimum(this->_crossEquities().minAtom());
            spin->setMaximum(this->_crossEquities().maxAtom());

            QObject::connect(
                spin, qOverload<double>(&QDoubleSpinBox ::valueChanged),
                this, &NonLinearAtomSliderEditor::_onSpinBoxValueChanged
            );

            return spin;

        }

        void _updateSpinner(double toApply) override {
            QSignalBlocker l(this->_spin);
            ((QDoubleSpinBox*)this->_spin)->setValue(toApply);
        }

    
 private:
        void _onSpinBoxValueChanged(double spinValue) {
            auto sliderVal = this->toSliderValue(spinValue);
            this->_updateSlider(sliderVal);
            this->_triggerAlterations();
        }
};