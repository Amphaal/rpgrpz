#pragma once

#include <QSlider>

#include "src/helpers/AtomConverter.h"
#include "src/ui/atoms/base/AtomSubEditor.h"

class AtomSliderEditor : public AtomSubEditor {
    public:
        AtomSliderEditor(const AtomParameter &parameter, int minimum, int maximum);
        void loadTemplate(QVector<const RPZAtom*> &atomsToManipulate, QVariant &defaultValue) override;
        QSlider* slider();

    private:
        void _onSliderChanging(int sliderVal);

        double outputValue();
        int atomValueToSliderValue(double atomValue);
    
    protected: 
        virtual double _toAtomValue(int sliderVal);
        virtual int _toSliderValue(double atomValue);
};