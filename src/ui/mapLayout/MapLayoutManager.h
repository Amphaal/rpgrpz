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