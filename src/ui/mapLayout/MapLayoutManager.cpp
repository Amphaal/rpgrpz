#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QGraphicsScene* scene, AtomsStorage* mapMaster, QWidget *parent) : QWidget(parent) {

    this->_tree = new MapLayoutTree(mapMaster, this);
    this->_layerSelector = new LayerSelector(this);
    this->_minimap = new MiniMapView(scene);
    
    auto layout = new QVBoxLayout;
    this->setLayout(layout);
    layout->addWidget(this->_layerSelector);
    layout->addWidget(this->_tree, 1);
    layout->addWidget(this->_minimap, 0, Qt::AlignCenter);

    layout->setSpacing(2);
    layout->setMargin(5);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
}

MapLayoutTree* MapLayoutManager::tree() {
    return this->_tree;
}

LayerSelector* MapLayoutManager::layerSelector(){
    return this->_layerSelector;
}

MiniMapView* MapLayoutManager::minimap() {
    return this->_minimap;
}