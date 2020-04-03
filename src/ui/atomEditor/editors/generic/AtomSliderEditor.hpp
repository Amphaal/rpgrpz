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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "src/ui/atomEditor/_base/AbstractAtomSliderEditor.h"

class AtomSliderEditor : public AbstractAtomSliderEditor {
    public:
        AtomSliderEditor(const RPZAtom::Parameter &parameter, int minimum, int maximum) : 
            AbstractAtomSliderEditor(parameter, QVector<CrossEquities::CrossEquity> { {(double)minimum, minimum}, {(double)maximum, maximum} }) {
                this->_spin = this->_generateSpinBox();
                this->_widgetLineLayout->addWidget(this->_spin);
            }

    protected: 
        QAbstractSpinBox* _generateSpinBox() const override {
            
            auto spin = new QSpinBox;
 
            spin->setMinimum(this->_crossEquities().minSlider());
            spin->setMaximum(this->_crossEquities().maxSlider());

            QObject::connect(
                spin, qOverload<int>(&QSpinBox ::valueChanged),
                this, &AtomSliderEditor::_onSpinnerValueChanged
            );

            return spin;

        }

        void _updateSpinner(double toApply) override {
            QSignalBlocker l(this->_spin);
            ((QSpinBox*)this->_spin)->setValue((int)toApply);
        }

    private:
        void _onSpinnerValueChanged(int atomValue) {
            auto sliderVal = this->toSliderValue(atomValue);
            this->_updateSlider(sliderVal);
            this->_triggerAlterations();
        }

};