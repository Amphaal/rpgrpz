#pragma once

#include "src/ui/atoms/base/AtomSliderEditor.hpp"

class AtomScalingEditor : public AtomSliderEditor {
    
    public:
        AtomScalingEditor(AtomEditor* parent) : AtomSliderEditor("Taille:", "x", parent) {
            this->_slider->setMinimum(1);
            this->_slider->setMaximum(1000);
        }

    private:
        double outputValue() override {
            return this->_toScaleValue(
                AtomSliderEditor::outputValue()
            );
        }

        double atomValue(RPZAtom &atom) override {
            return atom.scale();
        }

        int atomValueToSliderValue(RPZAtom &atom) override {
            return this->_fromScaleValue(
                this->atomValue(atom)
            );
        }

        AlterationPayload _createPayload(QVector<snowflake_uid> &toAlter, double newValue) override {
            return ScaledPayload(toAlter, newValue);
        }

        void _updateGraphicsItem(QGraphicsItem* giToUpdate, double value) override {
            giToUpdate->setScale(value);
        }

        //
        //
        //

        //https://www.wolframalpha.com/input/?i=fit+exponential++%5B(1,+0.1),+(500,+1),+(1000,+10)%5D
        double _toScaleValue(int sliderVal) {
            auto eqResult = (double).0999821 * qExp((double)0.00460535 * sliderVal);
            return round(eqResult * 100) / 100;
        }

        //https://www.wolframalpha.com/input/?i=fit+log+%5B(.1,+1),+(1,+500),+(10,+1000)%5D
        int _fromScaleValue(double scaled) {
            auto eqResult = (double)216.93 * qLn((double)10.0385 * scaled);
            auto roundedResult = round(eqResult * 100) / 100;
            return roundedResult < 1 ? 1 : roundedResult;
        }
};