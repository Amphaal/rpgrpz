// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

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
        QHBoxLayout* _widgetLineLayout = nullptr;
        QAbstractSpinBox* _spin = nullptr;
        virtual QAbstractSpinBox* _generateSpinBox() const = 0;

        void _onSliderValueChanged(int sliderVal);

        const CrossEquities& _crossEquities() const;
        
        void _updateSlider(int toApply);
        virtual void _updateSpinner(double toApply) = 0;
        void _initialSetup(double atomVal);

        void _triggerAlterations();
        

};