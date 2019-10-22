#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QGraphicsView* viewToMimic, AtomsStorage* mapMaster, QWidget *parent) : QWidget(parent) {

    this->_tree = new MapLayoutTree(mapMaster, this);
    this->_layerSelector = new LayerSelector(this);
    
    auto layout = new QVBoxLayout;
    this->setLayout(layout);
    layout->addWidget(this->_layerSelector);
    layout->addWidget(this->_tree, 1);

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