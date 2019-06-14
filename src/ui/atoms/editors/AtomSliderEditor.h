#pragma once

#include <QSlider>

#include "src/ui/atoms/base/AtomSubEditor.h"

class AtomSliderEditor : public AtomSubEditor {
    public:
        AtomSliderEditor(const AtomParameter &parameter, int minimum, int maximum);

        QSlider* slider();

    private:
        void loadTemplate(QVector<RPZAtom*> &atomsToManipulate, QVariant &defaultValue) override;

        void _onSliderChanging(int sliderVal);

        double outputValue();
        int atomValueToSliderValue(double atomValue);
    
    protected: 
        virtual double _toAtomValue(int sliderVal);
        virtual int _toSliderValue(double atomValue);
};