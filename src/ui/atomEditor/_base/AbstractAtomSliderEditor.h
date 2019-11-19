#pragma once

#include "src/shared/renderer/AtomConverter.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/atomEditor/_base/CrossEquities.hpp"

#include <QTimer>
#include <QSlider>
#include <QSpinBox>

class AbstractAtomSliderEditor : public AtomSubEditor {
    public:
        AbstractAtomSliderEditor(const RPZAtom::Parameter &parameter, const CrossEquities &crossEquities);
                
        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override;
        
        double outputValue() const;
        double toAtomValue(int sliderVal) const;
        int toSliderValue(double atomVal) const;

    private:
        CrossEquities _ceData;
        
        void _confirmPayload();
        void _confirmPreview();

        QTimer _commitTimer;
        QSlider* _slider = nullptr;

    protected: 
        QAbstractSpinBox* _spin = nullptr;
        const CrossEquities& _crossEquities() const;

        virtual QAbstractSpinBox* _generateSpinBox() const = 0;
        
        virtual void _updateWidgetsFromSliderVal(int sliderVal);
        void _updateWidgetsFromAtomVal(double atomVal);
        
        void _onSliderValueChanged(int sliderVal);

};