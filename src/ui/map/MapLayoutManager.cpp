#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QWidget *parent) : QWidget(parent), 
_tree(new MapLayoutTree(this)), 
_layerSelector(new LayerSelector(this)) {
        
    this->setLayout(new QVBoxLayout);
    this->layout()->addWidget(this->_layerSelector);
    this->layout()->addWidget(this->_tree);

    this->layout()->setSpacing(2);
    this->layout()->setMargin(5);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
}

MapLayoutTree* MapLayoutManager::tree() {
    return this->_tree;
}

LayerSelector* MapLayoutManager::layerSelector(){
    return this->_layerSelector;
}
