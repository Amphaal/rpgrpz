#pragma once

#include "AtomSliderEditor.h"

#include <QtMath>

class NonLinearAtomSliderEditor : public AtomSliderEditor {
    public:
        NonLinearAtomSliderEditor(const AtomParameter &parameter, int minimum, int maximum) : AtomSliderEditor(parameter, minimum, maximum) {}

    private:    
        //https://www.wolframalpha.com/input/?i=fit+exponential++%5B(1,+0.1),+(500,+1),+(1000,+10)%5D
        double _toAtomValue(int sliderVal) override {
            auto eqResult = (double).0999821 * qExp((double)0.00460535 * sliderVal);
            return round(eqResult * 100) / 100;
        }

        //https://www.wolframalpha.com/input/?i=fit+log+%5B(.1,+1),+(1,+500),+(10,+1000)%5D
        int _toSliderValue(double atomValue) override {
            auto eqResult = (double)216.93 * qLn((double)10.0385 * atomValue);
            auto roundedResult = round(eqResult * 100) / 100;
            return roundedResult < 1 ? 1 : roundedResult;
        }
};