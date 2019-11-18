#pragma once

#include "src/shared/renderer/AtomConverter.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"

#include <QTimer>
#include <QSlider>
#include <QSpinBox>

class AtomSliderEditor : public AtomSubEditor {
    public:
        AtomSliderEditor(const RPZAtom::Parameter &parameter, int minimum, int maximum);
        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override;

    private:
        void _onValueChanged(int sliderVal);
        
        void _confirmPayload();
        void _confirmPreview();

        double outputValue();
        int atomValueToSliderValue(double atomValue);

        QTimer _commitTimer;
        QSlider* _slider = nullptr;
        QSpinBox* _spin = nullptr;

        void _internalWidgetsUpdate(int sliderVal);

    protected: 
        virtual double _toAtomValue(int sliderVal);
        virtual int _toSliderValue(double atomValue);
};