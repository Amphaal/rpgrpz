#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

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

        RPZMapParameters _currentMapParameters;

        void _handleMapParametersEdition();
};