#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(AtomsStorage* mapMaster, QWidget * parent) : RPZTree(parent) {
    
    this->_hints = new TreeMapHint;
    this->_menuHandler = new AtomsContextualMenuHandler(mapMaster, this);
    
    this->_hints->moveToThread(new QThread);
    this->_hints->thread()->setObjectName(QStringLiteral(u"TreeLayoutThread"));
    this->_hints->thread()->start();

    this->setHeaderHidden(true);
    this->setUniformRowHeights(true);
	this->setSortingEnabled(true);

    this->header()->setSortIndicatorShown(false);
    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);
    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    this->setColumnCount(3);
    this->setItemDelegateForColumn(1, new LockAndVisibilityDelegate);
    this->setItemDelegateForColumn(2, new OwnerDelegate);
    
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    this->_handleHintsSignalsAndSlots();

}

MapLayoutTree::~MapLayoutTree() {
    if(this->_hints) {
        this->_hints->thread()->quit();
        this->_hints->thread()->wait();
    }
}

void MapLayoutTree::_handleHintsSignalsAndSlots() {

    //on map loading, disable
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessing,
        [=]() {
            this->setEnabled(false);
        }
    );

    //on std alteration requested
    QObject::connect(
        this->_hints, &TreeMapHint::requestingUIAlteration,
        this, &MapLayoutTree::_onUIAlterationRequest
    );

    //move requested
    QObject::connect(
        this->_hints, &TreeMapHint::requestingUIMove,
        this, &MapLayoutTree::_onUIMoveRequest
    );

    //owner change
    QObject::connect(
        this->_hints, &TreeMapHint::requestingUIUserChange,
        this, &MapLayoutTree::_onUIUserChangeRequest
    );

    //on updates
    QObject::connect(
        this->_hints, QOverload<const QList<QTreeWidgetItem*>&, const AtomUpdates&>::of(&TreeMapHint::requestingUIUpdate),
        this, QOverload<const QList<QTreeWidgetItem*>&, const AtomUpdates&>::of(&MapLayoutTree::_onUIUpdateRequest)
    );

    //on updates (bulk)
    QObject::connect(
        this->_hints, QOverload<const QHash<QTreeWidgetItem*, AtomUpdates>&>::of(&TreeMapHint::requestingUIUpdate),
        this, QOverload<const QHash<QTreeWidgetItem*, AtomUpdates>&>::of(&MapLayoutTree::_onUIUpdateRequest)
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

            //clear focus if empty
            auto selected = this->selectedItems();
            if(!selected.count()) this->clearFocus();

            //restrict list of selected item to unlocked ones
            QList<QTreeWidgetItem *> filtered;
            for(auto i : selected) {
                if(this->_isAssociatedAtomSelectable(i)) filtered += i;
            }

            //propagate with filtered list
            if(!filtered.isEmpty()) {
                auto filteredIds = _extractRPZAtomIdFromItems(filtered);
                this->_hints->propagateSelection(filteredIds);
            }

        }
    );

}

bool MapLayoutTree::_isAssociatedAtomSelectable(QTreeWidgetItem* item) {
    return !item->data(1, RPZUserRoles::AtomAvailability).toBool();
}

void MapLayoutTree::_onUIAlterationRequest(const PayloadAlteration &type, const QList<QTreeWidgetItem*> &toAlter) {
    
    //prevent circual selection/focus
    QSignalBlocker b(this);

    if(type == PA_Selected || type == PA_Focused) this->_clearSelectedItems();
    if(type == PA_Reset) this->clear();

    for(auto item : toAlter) {
        switch(type) {
            
            case PA_Selected:
                this->_selectAtomItem(item);
            break;

            case PA_Focused: {
                auto itemIndex = this->indexFromItem(item);
                this->scrollTo(itemIndex, QAbstractItemView::ScrollHint::PositionAtCenter);
                this->_selectAtomItem(item);
            }
            break;

            case PA_Added:
            case PA_Reset:
                this->_insertAtomItem(item);
            break;

            case PA_Removed:
                this->_removeItem(item);
            break;

            default:
            break;

        }
    }

    //in case of disabling from heavy alteration
    this->setEnabled(true); 
    
    if(type == PA_Reset || type == PA_Added) this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
    if(type == PA_Reset || type == PA_Added || type == PA_Removed) this->_updateLayersDisplayedCount();

    this->_resizeSections();

}

void MapLayoutTree::_resizeSections() {
    this->header()->resizeSections(QHeaderView::ResizeMode::ResizeToContents);
}

void MapLayoutTree::_onUIUpdateRequest(const QHash<QTreeWidgetItem*, AtomUpdates> &toUpdate) {
    
    for(auto i = toUpdate.constBegin(); i != toUpdate.constEnd(); i++) {
        this->_updateAtomItemValues(i.key(), i.value());
    }

    this->_resizeSections();

}

void MapLayoutTree::_onUIUpdateRequest(const QList<QTreeWidgetItem*> &toUpdate, const AtomUpdates &updates) {
    
    for(auto item : toUpdate) {
        this->_updateAtomItemValues(item, updates);
    }

    this->_resizeSections();

}

void MapLayoutTree::_onUIUserChangeRequest(const QList<QTreeWidgetItem*> &toUpdate, const RPZUser &newUser) {
    this->_hints->updateOwnerFromItems(toUpdate.toVector(), newUser);
    this->_resizeSections();
}

void MapLayoutTree::_onUIMoveRequest(const QHash<int, QList<QTreeWidgetItem*>> &childrenMovedToLayer) {
    
    //iterate moves by layer
    for(auto i = childrenMovedToLayer.constBegin(); i != childrenMovedToLayer.constEnd(); i++) {
        
        auto layerToMoveTo = i.key();
        auto widgetsToMove = i.value();
        auto layerItem = this->_hints->getLayerItem(layerToMoveTo);
        this->addTopLevelItem(layerItem); //try to add target layer, does nothing if already exists

        //for each widget to move in the targeted layer
        for(auto child : widgetsToMove) {
            auto sourceLayerItem = child->parent();
            sourceLayerItem->removeChild(child);
            layerItem->addChild(child);
        }

    }

    this->_updateLayersDisplayedCount();
    this->_resizeSections();

}

void MapLayoutTree::_updateAtomItemValues(QTreeWidgetItem* toUpdate, const AtomUpdates &updates) {
    
    for(auto i = updates.constBegin(); i != updates.constEnd(); i++) {
        switch(i.key()) {
            
            case AtomParameter::AssetName:
                this->_renameAtomItem(toUpdate, i.value().toString());
            break;
            
            case AtomParameter::Hidden: {
                auto isHidden = i.value().toBool();
                toUpdate->setData(1, RPZUserRoles::AtomVisibility, isHidden);
            }
            break;

            case AtomParameter::Locked: {
                auto isLocked = i.value().toBool();
                this->_hints->updateLockedState(toUpdate, isLocked);
            }
            break;

            default:
            break;
        }
    }

    this->_resizeSections();

}

void MapLayoutTree::_insertAtomItem(QTreeWidgetItem *item) {
    auto layer = item->data(0, RPZUserRoles::AtomLayer).toInt();
    auto layerItem = this->_hints->getLayerItem(layer);
    this->addTopLevelItem(layerItem); //try to add target layer, does nothing if already exists
    layerItem->addChild(item);
}

void MapLayoutTree::_clearSelectedItems() {
    this->clearSelection();
}

void MapLayoutTree::_selectAtomItem(QTreeWidgetItem* toSelect) {
    toSelect->setSelected(true);
}

void MapLayoutTree::_removeItem(QTreeWidgetItem* toRemove) {
    if(auto maybeLayerItem = toRemove->parent()) {
        maybeLayerItem->removeChild(toRemove);
    }
    else {
        auto topLevelIndex = this->indexOfTopLevelItem(toRemove);
        this->takeTopLevelItem(topLevelIndex);
    }
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

TreeMapHint* MapLayoutTree::hints() const {
    return this->_hints;
}

void MapLayoutTree::contextMenuEvent(QContextMenuEvent *event) {

    auto ids = this->_extractRPZAtomIdFromItems(
        this->selectedItems()
    );

    //create menu
    this->_menuHandler->invokeMenu(ids, event->globalPos());

}


void MapLayoutTree::keyPressEvent(QKeyEvent * event) {
    
    RPZTree::keyPressEvent(event);

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            this->_menuHandler->removeSelectedAtoms();
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
