#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "MapLayoutTree.h"
#include "base/LayerSelector.h"


class MapLayoutManager : public QWidget {
    public:
        MapLayoutManager(QGraphicsView* viewToMimic, AtomsStorage* mapMaster, QWidget *parent = nullptr);

        MapLayoutTree* tree();
        LayerSelector* layerSelector();
    
    private:
        MapLayoutTree* _tree = nullptr;
        LayerSelector* _layerSelector = nullptr;
};