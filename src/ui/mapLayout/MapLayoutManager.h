#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "MapLayoutTree.h"
#include "src/ui/map/MapParametersForm.hpp"

class MapLayoutManager : public QWidget {
    public:
        MapLayoutManager(QGraphicsView* viewToMimic, AtomsStorage* mapMaster, QWidget *parent = nullptr);

        MapLayoutTree* tree();
    
    private:
        MapLayoutTree* _tree = nullptr;
        QPushButton* _mapParamBtn = nullptr;
        AtomsStorage* _mapMaster = nullptr;

        RPZMapParameters _currentMapParameters;

        void _handleMapParametersEdition();
};