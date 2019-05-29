#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QWidget * parent) : RPZTree(parent) {
    
    this->setColumnCount(2);

    this->setHeaderHidden(true);
    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);
    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    QObject::connect(
        this, &QTreeWidget::itemSelectionChanged,
        this, &MapLayoutManager::_onElementSelectionChanged
    );

    QObject::connect(
        this, &QTreeWidget::itemDoubleClicked,
        this, &MapLayoutManager::_onElementDoubleClicked
    );

    //context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(
        this, &QWidget::customContextMenuRequested,
        this, &MapLayoutManager::_renderCustomContextMenu
    );
}

void MapLayoutManager::_renderCustomContextMenu(const QPoint &pos) {
    
    auto itemsToProcess = this->selectedItems();

    //check selected items (autoselected on right click)
    if(!itemsToProcess.count()) {
        //get elem under cursor
        auto item = this->itemAt(pos);
        if(item) {
            //prevent usage of layer item
            if(item->parent()) {
                itemsToProcess.append(item);
                item->setSelected(true);
            }
        }
    }

    //create menu
    this->_generateMenu(itemsToProcess, this->viewport()->mapToGlobal(pos));
}

void MapLayoutManager::_generateMenu(QList<QTreeWidgetItem*> &itemsToProcess, const QPoint &whereToDisplay) {
    
    //if still no item, stop
    if(!itemsToProcess.count()) return;

    //targets
    auto riseLayoutTarget = itemsToProcess.first()->parent()->data(0, Qt::UserRole).toInt() + 1;
    auto lowerLayoutTarget = itemsToProcess.last()->parent()->data(0, Qt::UserRole).toInt() - 1;

    //list of actions to bind to menu
    QList<QAction*> actions;
    
        //rise...
        auto riseAction = new QAction("Remonter (Calque " + QString::number(riseLayoutTarget) + ")");
        QObject::connect(
            riseAction, &QAction::triggered,
            [=]() {this->_moveSelectionToLayer(riseLayoutTarget);}
        );
        actions.append(riseAction);

        //lower...
        auto lowerAction = new QAction("Descendre (Calque " + QString::number(lowerLayoutTarget) + ")");
        QObject::connect(
            lowerAction, &QAction::triggered,
            [=]() {this->_moveSelectionToLayer(lowerLayoutTarget);}
        );
        actions.append(lowerAction);

    //display menu
    QMenu menu;
    menu.addActions(actions);
    menu.exec(whereToDisplay);
}

void MapLayoutManager::_moveSelectionToLayer(int targetLayer) {

    auto selectedIds = this->_extractIdsFromSelection();
    this->_changeLayer(selectedIds, targetLayer);

    //unilateral event, expect only outer calls
    this->_expectedPingback = RPZAsset::Alteration::Unknown;
    emit elementsAlterationAsked(this->_expectedPingback, selectedIds, QVariant(targetLayer));
}


void MapLayoutManager::_onElementDoubleClicked(QTreeWidgetItem * item, int column) {
    emit elementsAlterationAsked(RPZAsset::Alteration::Focused, this->_extractIdsFromSelection());
}

void MapLayoutManager::_onElementSelectionChanged() {
    
    auto associatedAlteration = RPZAsset::Alteration::Selected;

    //bilateral event, prevent circular for inner and outer events
    if(this->_expectedPingback == associatedAlteration) return;

    this->_expectedPingback = associatedAlteration;
    emit elementsAlterationAsked(associatedAlteration, this->_extractIdsFromSelection());

}

void MapLayoutManager::alterTreeElements(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements) {
    
    //if pingback invocation, skip
    if(this->_expectedPingback == state) {
        this->_expectedPingback = RPZAsset::Alteration::Unknown;
        return;
    }

    //special handling
    if(state == RPZAsset::Alteration::Selected) {
        this->_expectedPingback = state; //prevent inner circular event calls
        this->clearSelection();
    }

    //special handling
    else if(state == RPZAsset::Alteration::Reset) {
        
        //empty
        for(auto item : this->_treeItemsById) {
            delete item;
        }

        this->_treeItemsById.clear();
    }

    //iterate through items
    for (auto &e : elements) {

        const auto key = e.id();

        switch(state) {

            case RPZAsset::Alteration::Removed: {
                    auto item = this->_treeItemsById[key];
                    if(item) {

                        auto layerItem = item->parent();
                        delete this->_treeItemsById.take(key);

                        //also remove layer
                        this->_updateLayerState(layerItem);
                    }
                }
                break;

            case RPZAsset::Alteration::LayerChange: {
                    this->_changeLayer(e);
                }
                break;

            case RPZAsset::Alteration::Selected: {
                    auto item = this->_treeItemsById[key];
                    if(item) item->setSelected(true);
                }
                break;

            case RPZAsset::Alteration::Reset:
            case RPZAsset::Alteration::Added: {
                    auto item = this->_createTreeItem(e);
                    this->_treeItemsById.insert(key, item);
                }
                break;
        }
    }

    //release lock
    if(state == RPZAsset::Alteration::Selected) {
        this->_expectedPingback = RPZAsset::Alteration::Unknown;
    }

}

void MapLayoutManager::_changeLayer(RPZAsset &asset) {
    QVector<QUuid> arg;
    arg.append(asset.id());
    this->_changeLayer(arg, asset.metadata()->layer());
}

void MapLayoutManager::_changeLayer(QVector<QUuid> &elementIds, int newLayer) {

    QSet<QTreeWidgetItem*> dirtyLayerItems;

    //inner elements to update
    for(auto &key : elementIds) {
        auto item = this->_treeItemsById[key];
        if(item) {
            
            //remove from initial layer, maybe remove it too
            auto oldLayerItem = item->parent();
            oldLayerItem->removeChild(item);

            //go to the other layer
            auto newLayerItem = this->_getLayerItem(newLayer);
            newLayerItem->addChild(item);

            //add dirty
            dirtyLayerItems.insert(oldLayerItem);
            dirtyLayerItems.insert(newLayerItem);
        }
    }

    //update layer items
    for(auto item : dirtyLayerItems) {
        this->_updateLayerState(item);
    }


}

QTreeWidgetItem* MapLayoutManager::_getLayerItem(RPZAsset &asset) {
    return this->_getLayerItem(asset.metadata()->layer());
}

QTreeWidgetItem* MapLayoutManager::_getLayerItem(int layer) {
    
    auto layerElem = this->_layersItems[layer];
    
    if(!layerElem) {

        //define new
        layerElem = new QTreeWidgetItem();
        layerElem->setText(0, "Calque " + QString::number(layer));
        layerElem->setData(0, Qt::UserRole, QVariant(layer));
        layerElem->setIcon(0, QIcon(":/icons/app/manager/layer.png"));
        layerElem->setFlags(Qt::ItemIsEnabled);
        
        //add to layout
        this->_layersItems[layer] = layerElem;
        this->addTopLevelItem(layerElem);

        //sort
        this->setSortingEnabled(true);
        this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
        this->setSortingEnabled(false);
    }

    return layerElem;
}

QTreeWidgetItem* MapLayoutManager::_createTreeItem(RPZAsset &asset) {
    
    auto item = new QTreeWidgetItem();
    item->setText(0, asset.descriptor());
    item->setData(0, Qt::UserRole, asset.id());
    item->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled | 
            Qt::ItemNeverHasChildren | 
            Qt::ItemIsSelectable
        )
    );

    const auto type = asset.type();
    switch(type) {
        case AssetBase::Type::Drawing:
            item->setIcon(0, QIcon(":/icons/app/manager/drawing.png"));
            break;
    }

    //create or get the layer element
    auto layerElem = this->_getLayerItem(asset);
    layerElem->addChild(item);
    this->_updateLayerState(layerElem);

    return item;
}

void MapLayoutManager::_updateLayerState(QTreeWidgetItem* layerItem) {
    if(layerItem->childCount()) {
        //has children, update count column
        layerItem->setText(1, QString::number(layerItem->childCount()));
        layerItem->setExpanded(true);
    } else {
        //has no more children, remove
        auto layer = layerItem->data(0, Qt::UserRole).toInt();
        delete this->_layersItems.take(layer);
    }
}

void MapLayoutManager::keyPressEvent(QKeyEvent * event) {
    
    RPZTree::keyPressEvent(event);

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:

            //make sure there is a selection
            const auto selectedIds = this->_extractIdsFromSelection();
            if(!selectedIds.length()) return;

            emit elementsAlterationAsked(RPZAsset::Alteration::Removed, selectedIds);
            break;
    }

}

QVector<QUuid> MapLayoutManager::_extractIdsFromSelection() const {
    
    QVector<QUuid> idList;
    
    for(auto &i : this->selectedItems()) {
        const auto innerData = i->data(0, Qt::UserRole).toUuid();
        idList.append(innerData);
    }

    return idList;
}
