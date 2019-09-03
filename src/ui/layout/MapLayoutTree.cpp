#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(AtomsStorage* mapMaster, QWidget * parent) : 
    RPZTree(parent), 
    AtomsContextualMenuHandler(mapMaster, this),
    _hints(new TreeMapHint) {
        
    this->_hints->moveToThread(new QThread);

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

    //on std alteration requested
    QObject::connect(
        this->_hints, &TreeMapHint::requestingUIAlteration,
        [=](const PayloadAlteration &type, const QList<QTreeWidgetItem*> &toAlter) {
            
            if(type == PA_Selected) this->_clearSelectedItems();
            if(type == PA_Reset) this->clear();

            for(auto item : toAlter) {
                switch(type) {
                    
                    case PA_Selected:
                        this->_selectAtomItem(item);
                    break;

                    case PA_Added:
                    case PA_Reset:
                        this->_insertAtomItem(item);
                    break;

                    case PA_Removed:
                        this->_removeItem(item);
                    break;


                }
            }

            if(type == PA_Reset || type == PA_Added) this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
            if(type == PA_Reset || type == PA_Added || type == PA_Removed) this->_updateLayersDisplayedCount();

        }
    );

    //move requested
    QObject::connect(
        this->_hints, &TreeMapHint::requestingUIMove,
        [=](const QHash<int, QList<QTreeWidgetItem*>> &childrenMovedToLayer) {

            for(auto i = childrenMovedToLayer.constBegin(); i != childrenMovedToLayer.constEnd(); i++) {
                for(auto child : i.value()) {
                    child->parent()->removeChild(child);
                    auto layerItem = this->_hints->getLayerItem(i.key());
                    layerItem->addChild(child);
                }
            }

            this->_updateLayersDisplayedCount();
        }
    );

    //owner change
    QObject::connect(
        this->_hints, &TreeMapHint::requestingUIUserChange,
        [=](const QList<QTreeWidgetItem*> &toUpdate, const RPZUser &newUser) {
            for(auto item : toUpdate) {
                this->_hints->updateOwnerFromItem(item, newUser);
            }
        }
    );

    //on updates
    QObject::connect(
        this->_hints, QOverload<const QList<QTreeWidgetItem*>&, const AtomUpdates&>::of(&TreeMapHint::requestingUIUpdate),
        [=](const QList<QTreeWidgetItem*> &toUpdate, const AtomUpdates &updates) {
            for(auto item : toUpdate) {
                this->_updateAtomItemValues(item, updates);
            }
        }
    );

    //on updates (bulk)
    QObject::connect(
        this->_hints, QOverload<const QHash<QTreeWidgetItem*, AtomUpdates>&>::of(&TreeMapHint::requestingUIUpdate),
        [=](const QHash<QTreeWidgetItem*, AtomUpdates> &toUpdate) { 
            for(auto i = toUpdate.constBegin(); i != toUpdate.constEnd(); i++) {
                this->_updateAtomItemValues(i.key(), i.value());
            }
        }
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

void MapLayoutTree::_updateAtomItemValues(QTreeWidgetItem* toUpdate, const AtomUpdates &updates) {
    for(auto i = updates.constBegin(); i != updates.constEnd(); i++) {
        switch(i.key()) {
            
            case AtomParameter::AssetName:
                this->_renameAtomItem(toUpdate, i.value().toString()); //TODO get atom.descriptor() ?
            break;
            
            case AtomParameter::Hidden:
                toUpdate->setData(1, RPZUserRoles::AtomVisibility, i.value().toBool());
            break;

            case AtomParameter::Locked:
                toUpdate->setData(1, RPZUserRoles::AtomAvailability, i.value().toBool());
            break;

        }
    }

}

void MapLayoutTree::_insertAtomItem(QTreeWidgetItem *item) {
    auto layer = item->data(0, RPZUserRoles::AtomLayer).toInt();
    auto layerItem = this->_hints->getLayerItem(layer);
    this->addTopLevelItem(layerItem);
    layerItem->addChild(item);
}

void MapLayoutTree::_clearSelectedItems() {
    this->clearSelection();
}

void MapLayoutTree::_selectAtomItem(QTreeWidgetItem* toSelect) {
    toSelect->setSelected(true);
}

void MapLayoutTree::_removeItem(QTreeWidgetItem* toRemove) {
    auto maybeLayerItem = toRemove->parent();
    if(maybeLayerItem) maybeLayerItem->removeChild(toRemove);
    else this->removeItemWidget(toRemove, 0);
}

void MapLayoutTree::_updateLayersDisplayedCount() {
    for(auto i = 0; i < this->topLevelItemCount(); i++) {
        auto layerItem = this->topLevelItem(i);
        auto count = layerItem->childCount();
        layerItem->setText(2, QString::number(count));
        layerItem->setExpanded(true);
    }
}

void MapLayoutTree::_renameAtomItem(QTreeWidgetItem* toRename, const QString &newName) {
    toRename->setText(0, newName);
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
