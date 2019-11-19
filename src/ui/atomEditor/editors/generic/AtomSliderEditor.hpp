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
                this, &AtomSliderEditor::_onSliderValueChanged
            );

            return spin;

        }

        void _updateWidgetsFromSliderVal(int sliderVal) override {

            AbstractAtomSliderEditor::_updateWidgetsFromSliderVal(sliderVal);

            QSignalBlocker l(this->_spin);
            ((QSpinBox*)this->_spin)->setValue(sliderVal);

        }

};