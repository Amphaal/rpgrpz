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

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QHBoxLayout>
#include <QGroupBox>

#include "MapLayoutTree.h"
#include "src/ui/map/MapParametersForm.hpp"

#include "src/shared/hints/HintThread.hpp"

class MapLayoutManager : public QWidget {
    public:
        MapLayoutManager(QGraphicsView* viewToMimic, QWidget *parent = nullptr);

        MapLayoutTree* tree();
    
    private:
        MapLayoutTree* _tree = nullptr;
        QPushButton* _mapParamBtn = nullptr;
        QSlider* _FoWOpacitySlider = nullptr;
        QCheckBox* _FoWReversedChk = nullptr;
        QPushButton* _FoWResetBtn = nullptr;

        RPZMapParameters _currentMapParameters;

        void _handleMapParametersEdition();
        void _changeFogMode(int);
        void _fogOpacityChange(int);
        void _onMapSetup(const RPZMapParameters &mParams, const RPZFogParams &fParams);
        void _handleFogReset();
};