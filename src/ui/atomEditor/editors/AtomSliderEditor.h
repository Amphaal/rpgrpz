#pragma once

#include "src/shared/renderer/AtomConverter.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/atomEditor/editors/custom/RPZCustomSlider.hpp"

class AtomSliderEditor : public AtomSubEditor {
    public:
        AtomSliderEditor(const RPZAtom::Parameter &parameter, int minimum, int maximum);
        const AtomSubEditor::DefaultValues loadTemplate(const RPZAtom::Updates &defaultValues, bool updateMode) override;
        QSlider* slider();

    private:
        void _onSliderChanging(int sliderVal);

        double outputValue();
        int atomValueToSliderValue(double atomValue);

    protected: 
        virtual double _toAtomValue(int sliderVal);
        virtual int _toSliderValue(double atomValue);
};