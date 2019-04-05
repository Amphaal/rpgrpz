#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QWidget * parent) : QTreeWidget(parent) {
    this->setHeaderHidden(true);
    this->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
}