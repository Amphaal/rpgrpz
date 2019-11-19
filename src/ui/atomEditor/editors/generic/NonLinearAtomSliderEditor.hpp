#pragma once

#include "src/ui/atomEditor/_base/AbstractAtomSliderEditor.h"

#include <QtMath>
#include <QDoubleSpinBox>

class NonLinearAtomSliderEditor : public AbstractAtomSliderEditor {
    public:
        NonLinearAtomSliderEditor(const RPZAtom::Parameter &parameter, const CrossEquities &crossEquities) : AbstractAtomSliderEditor(parameter, crossEquities) {}

    protected: 
        QAbstractSpinBox* _generateSpinBox() const override {
            
            auto spin = new QDoubleSpinBox;
 
            spin->setMinimum(this->_crossEquities().v().first().atomValue);
            spin->setMaximum(this->_crossEquities().v().last().atomValue);

            QObject::connect(
                spin, qOverload<double>(&QDoubleSpinBox ::valueChanged),
                this, &NonLinearAtomSliderEditor::_onSpinBoxValueChanged
            );

            return spin;

        }

        void _updateWidgetsFromSliderVal(int sliderVal) override {

            AbstractAtomSliderEditor::_updateWidgetsFromSliderVal(sliderVal);

            auto atomVal = this->toAtomValue(sliderVal);

            QSignalBlocker l(this->_spin);
            ((QDoubleSpinBox*)this->_spin)->setValue(atomVal);
            
        }

    
    private:
        void _onSpinBoxValueChanged(double spinValue) {
            auto sliderVal = this->toSliderValue(spinValue);
            this->_onSliderValueChanged(sliderVal);
        }
};