#pragma once

#include "src/ui/atomEditor/editors/generic/AtomSliderEditor.h"

class OpacitySlider : public AtomSliderEditor {
    
    public:
        OpacitySlider() : AtomSliderEditor(RPZAtom::Parameter::Opacity, 50, 100) {}

    protected:
        double _toAtomValue(int sliderVal) override {
            return (double)sliderVal / 100;
        }

        int _toSliderValue(double atomValue) override {
            return (int)atomValue * 100;
        }

};