#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(AtomsStorage* mapMaster, QWidget * parent) : 
    RPZTree(parent), 
    AtomsContextualMenuHandler(mapMaster, this),
    _hints(new TreeMapHint) {
        
	this->setSortingEnabled(true);

    this->setItemDelegateForColumn(1, new LockAndVisibilityDelegate);
    this->setItemDelegateForColumn(2, new OwnerDelegate);

    this->setColumnCount(3);

    this->setHeaderHidden(true);
    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);

    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);

    this->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    this->_handleHintsSignalsAndSlots();

}

void MapLayoutTree::_handleHintsSignalsAndSlots() {
    
    //on insertion required
    QObject::connect(
        this->_hints, &TreeMapHint::requestingItemInsertion,
        this, &MapLayoutTree::_insertTreeWidgetItem
    );

    //clear wanted
    QObject::connect(
        this->_hints, &TreeMapHint::requestingItemClearing,
        this, &QTreeWidget::clear
    );

    //move requested
    QObject::connect(
        this->_hints, &TreeMapHint::requestingItemMove,
        this, &MapLayoutTree::_moveFromLayer
    );

    //on rename
    QObject::connect(
        this->_hints, &TreeMapHint::requestingItemTextChange,
        this, &MapLayoutTree::_renameItem
    );

    //on specific remove
    QObject::connect(
        this->_hints, &TreeMapHint::requestingItemDeletion,
        this, &MapLayoutTree::_removeItem
    );

    //on selection
    QObject::connect(
        this->_hints, &TreeMapHint::requestingItemSelection,
        this, &MapLayoutTree::_selectItem
    );

    //on clear selection
    QObject::connect(
        this->_hints, &TreeMapHint::requestingItemSelectionClearing,
        this, &MapLayoutTree::_clearSelectedItems
    );

    //on data change request
    QObject::connect(
        this->_hints, &TreeMapHint::requestingItemDataUpdate,
        this, &MapLayoutTree::_changeItemData
    );
    
    //focus
    QObject::connect(
        this, &QTreeWidget::itemDoubleClicked,
        [=](QTreeWidgetItem *item, int column) {
            auto focusedRPZAtomId = this->_extractRPZAtomIdFromItem(item);
            if(!focusedRPZAtomId) return;
            this->_hints->propagateFocus(focusedRPZAtomId);
        }
    );

    //selection
    QObject::connect(
        this, &QTreeWidget::itemSelectionChanged,
        [=]() {

            auto selected = this->selectedItems();
            if(!selected.count()) this->clearFocus();
            
            auto selectedIds = _extractRPZAtomIdFromItems(selected);
            this->_hints->propagateSelection(selectedIds);
        }
    );

}

void MapLayoutTree::_changeItemData(QTreeWidgetItem* target, int column, const QHash<int, QVariant> &newData) {
    for(QHash<int, QVariant>::const_iterator i = newData.begin(); i != newData.end(); ++i) {
        target->setData(column, i.key(), i.value());
    }
}


void MapLayoutTree::_insertTreeWidgetItem(QTreeWidgetItem *item, QTreeWidgetItem* parent) {
    if(!parent) {
        this->addTopLevelItem(item);
        this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
        this->_updateLayerState(item);
    } else {
        parent->addChild(item);
        this->_updateLayerState(parent);
    }
}

void MapLayoutTree::_clearSelectedItems() {
    this->clearSelection();
}

void MapLayoutTree::_selectItem(QTreeWidgetItem* toSelect) {
    toSelect->setSelected(true);
}

void MapLayoutTree::_removeItem(QTreeWidgetItem* toRemove) {
    auto layerItem = toRemove->parent();
    layerItem->removeChild(toRemove);
    this->_updateLayerState(layerItem);
}

void MapLayoutTree::_updateLayerState(QTreeWidgetItem* layerItem) {
    
    //has children, update count column
    if(auto childCount = layerItem->childCount()) {
        auto childCountStr = QString::number(childCount);
        layerItem->setText(2, childCountStr);
        layerItem->setExpanded(true);
    } 
    
    //has no more children, remove
    else {
        auto layer = layerItem->data(0, RPZUserRoles::AtomLayer).toInt();
        this->_hints->removeLayerItem(layer); 
    }

}

void MapLayoutTree::_renameItem(QTreeWidgetItem* toRename, const QString &newName) {
    toRename->setText(0, newName);
}

void MapLayoutTree::_moveFromLayer(QTreeWidgetItem* oldLayerItem, QTreeWidgetItem* newLayerItem, QTreeWidgetItem *item) {
    oldLayerItem->removeChild(item);
    newLayerItem->addChild(item);
    this->_updateLayerState(oldLayerItem);
    this->_updateLayerState(newLayerItem);
}


TreeMapHint* MapLayoutTree::hints() {
    return this->_hints;
}

void MapLayoutTree::contextMenuEvent(QContextMenuEvent *event) {
    
    auto pos = event->pos();
    auto itemsToProcess = this->selectedItems();

    auto count = itemsToProcess.count();
    if(!count) return;

    //targets
    auto riseLayoutTarget = itemsToProcess.first()->parent()->data(0, RPZUserRoles::AtomLayer).toInt() + 1;
    auto lowerLayoutTarget = itemsToProcess.last()->parent()->data(0, RPZUserRoles::AtomLayer).toInt() - 1;

    //create menu
    this->invokeMenu(riseLayoutTarget, lowerLayoutTarget, count, this->viewport()->mapToGlobal(pos));
}


void MapLayoutTree::keyPressEvent(QKeyEvent * event) {
    
    RPZTree::keyPressEvent(event);

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            this->removeSelectedAtoms();
            break;
    }

}


RPZAtomId MapLayoutTree::_extractRPZAtomIdFromItem(QTreeWidgetItem* item) const {
    return item->data(0, RPZUserRoles::AtomId).toULongLong();
}

QVector<RPZAtomId> MapLayoutTree::_extractRPZAtomIdFromItems(const QList<QTreeWidgetItem*> &items) const {
    QVector<RPZAtomId> idList;
    for(auto i : this->selectedItems()) {
        auto boundId = this->_extractRPZAtomIdFromItem(i);
        if(boundId) idList.append(boundId);
    }
    return idList;
}
