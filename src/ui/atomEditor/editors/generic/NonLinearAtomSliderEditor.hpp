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