#include "AssetsManager.h"

AssetsManager::AssetsManager(QWidget * parent) : QWidget(parent), 
    _tree(new AssetsTreeView) {

    this->setLayout(new QVBoxLayout);
    
    this->layout()->setMargin(5);
    this->layout()->setSpacing(2);

    this->layout()->addWidget(this->_tree);

    this->_tree->show();
    this->_tree->expandAll();
}

AssetsTreeView* AssetsManager::tree() {
    return this->_tree;
}