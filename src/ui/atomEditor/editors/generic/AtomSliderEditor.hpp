#pragma once

#include "src/ui/atomEditor/_base/AbstractAtomSliderEditor.h"

class AtomSliderEditor : public AbstractAtomSliderEditor {
    public:
        AtomSliderEditor(const RPZAtom::Parameter &parameter, int minimum, int maximum) : 
            AbstractAtomSliderEditor(parameter, QVector<CrossEquities::CrossEquity> { {minimum, minimum}, {maximum, maximum} }) {}

    protected: 
        QAbstractSpinBox* _generateSpinBox() const override {
            
            auto spin = new QSpinBox;
 
            spin->setMinimum(this->_crossEquities().v().first().sliderValue);
            spin->setMaximum(this->_crossEquities().v().last().sliderValue);

            QObject::connect(
                spin, qOverload<int>(&QSpinBox ::valueChanged),
                this, &AbstractAtomSliderEditor::_onSliderValueChanged
            );

            return spin;

        }

        void _updateWidgetsFromSliderVal(int sliderVal) override {

            AbstractAtomSliderEditor::_updateWidgetsFromSliderVal(sliderVal);

            QSignalBlocker l(this->_spin);
            ((QSpinBox*)this->_spin)->setValue(sliderVal);

        }

};