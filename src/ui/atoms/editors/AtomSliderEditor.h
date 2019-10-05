#pragma once

#include "src/helpers/AtomConverter.h"
#include "src/ui/atoms/base/AtomSubEditor.h"
#include "src/ui/atoms/editors/custom/RPZCustomSlider.hpp"

class AtomSliderEditor : public AtomSubEditor {
    public:
        AtomSliderEditor(const AtomParameter &parameter, int minimum, int maximum);
        QVariant loadTemplate(const AtomUpdates &defaultValues, bool updateMode) override;
        QSlider* slider();

    private:
        void _onSliderChanging(int sliderVal);

        double outputValue();
        int atomValueToSliderValue(double atomValue);

    protected: 
        virtual double _toAtomValue(int sliderVal);
        virtual int _toSliderValue(double atomValue);
};