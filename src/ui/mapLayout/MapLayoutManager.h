#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "MapLayoutTree.h"
#include "MiniMapView.hpp"
#include "base/LayerSelector.h"


class MapLayoutManager : public QWidget {
    public:
        MapLayoutManager(QGraphicsScene* scene, AtomsStorage* mapMaster, QWidget *parent = nullptr);

        MapLayoutTree* tree();
        LayerSelector* layerSelector();
        MiniMapView* minimap();
    
    private:
        MapLayoutTree* _tree = nullptr;
        LayerSelector* _layerSelector = nullptr;
        MiniMapView* _minimap = nullptr;
};