#pragma once

#include "src/ui/atoms/base/AtomSliderEditor.h"

class AtomScalingEditor : public AtomSliderEditor {
    
    public:
        AtomScalingEditor() : AtomSliderEditor("Taille:", "x", 1, 1000) {}

        AlterationPayload createPayload() override {
            return //MetadataChangedPayload(this->_atomsToSnowflakeList(), this->outputValue());
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