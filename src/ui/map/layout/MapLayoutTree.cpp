#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(QWidget * parent) : RPZTree(parent) {
    
    this->setColumnCount(2);

    this->setHeaderHidden(true);
    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);
    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    //selection changed
    QObject::connect(
        this, &QTreeWidget::itemSelectionChanged,
        this, &MapLayoutTree::_onElementSelectionChanged
    );

    //focus
    QObject::connect(
        this, &QTreeWidget::itemDoubleClicked,
        this, &MapLayoutTree::_onElementDoubleClicked
    );

    //context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(
        this, &QWidget::customContextMenuRequested,
        this, &MapLayoutTree::_renderCustomContextMenu
    );

    //on rename
    QObject::connect(
        AssetsDatabase::get(), &AssetsDatabase::assetRenamed,
        this, &MapLayoutTree::_onRenamedAsset
    );
}

void MapLayoutTree::_renderCustomContextMenu(const QPoint &pos) {
    
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

void MapLayoutTree::_generateMenu(QList<QTreeWidgetItem*> &itemsToProcess, const QPoint &whereToDisplay) {
    
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

void MapLayoutTree::_moveSelectionToLayer(int targetLayer) {

    auto selectedIds = this->_extractIdsFromSelection();
    this->_changeLayer(selectedIds, targetLayer);

    //unilateral event, expect only outer calls
    this->_expectedPingback = RPZAtom::Alteration::Unknown;
    emit elementsAlterationAsked(this->_expectedPingback, selectedIds, QVariant(targetLayer));
}


void MapLayoutTree::_onElementDoubleClicked(QTreeWidgetItem * item, int column) {
    emit elementsAlterationAsked(RPZAtom::Alteration::Focused, this->_extractIdsFromSelection());
}

void MapLayoutTree::_onElementSelectionChanged() {
    
    auto associatedAlteration = RPZAtom::Alteration::Selected;

    //bilateral event, prevent circular for inner and outer events
    if(this->_expectedPingback == associatedAlteration) return;

    this->_expectedPingback = associatedAlteration;
    emit elementsAlterationAsked(associatedAlteration, this->_extractIdsFromSelection());

}

void MapLayoutTree::alterTreeElements(const RPZAtom::Alteration &state, QVector<RPZAtom> &elements) {
    
    //if pingback invocation, skip
    if(this->_expectedPingback == state) {
        this->_expectedPingback = RPZAtom::Alteration::Unknown;
        return;
    }

    //special handling
    if(state == RPZAtom::Alteration::Selected) {
        this->_expectedPingback = state; //prevent inner circular event calls
        this->clearSelection();
    }

    //special handling
    else if(state == RPZAtom::Alteration::Reset) {
        
        //empty
        for(auto item : this->_treeItemsByAtomId) {
            delete item;
        }

        this->_treeItemsByAtomId.clear();
    }

    //iterate through items
    for (auto &e : elements) {

        const auto key = e.id();

        switch(state) {

            case RPZAtom::Alteration::Removed: {
                    auto item = this->_treeItemsByAtomId[key];
                    if(item) {

                        auto layerItem = item->parent();
                        auto oldItem = this->_treeItemsByAtomId.take(key);
                        delete oldItem;

                        //also remove layer
                        this->_updateLayerState(layerItem);

                        //if has assetId, remove it
                        auto assetId = e.metadata()->assetId();
                        if(!assetId.isNull()) {
                            this->_treeItemsByAssetId[assetId].remove(oldItem);
                        }
                    }
                }
                break;

            case RPZAtom::Alteration::LayerChange: {
                    this->_changeLayer(e);
                }
                break;

            case RPZAtom::Alteration::Selected: {
                    auto item = this->_treeItemsByAtomId[key];
                    if(item) item->setSelected(true);
                }
                break;

            case RPZAtom::Alteration::Reset:
            case RPZAtom::Alteration::Added: {
                    auto item = this->_createTreeItem(e);
                    this->_treeItemsByAtomId.insert(key, item);

                    //if has assetId, add it
                    auto assetId = e.metadata()->assetId();
                    if(!assetId.isNull()) {
                        this->_treeItemsByAssetId[assetId].insert(item);
                    }
                }
                break;
        }
    }

    //release lock
    if(state == RPZAtom::Alteration::Selected) {
        this->_expectedPingback = RPZAtom::Alteration::Unknown;
    }

}

void MapLayoutTree::_changeLayer(RPZAtom &atom) {
    QVector<QUuid> arg;
    arg.append(atom.id());
    this->_changeLayer(arg, atom.metadata()->layer());
}

void MapLayoutTree::_onRenamedAsset(const QString &assetId, const QString &newName) {
    if(!this->_treeItemsByAssetId.contains(assetId)) return;

    for(auto item : this->_treeItemsByAssetId[assetId]) {
        item->setText(0, newName);
    }
}

void MapLayoutTree::_changeLayer(QVector<QUuid> &elementIds, int newLayer) {

    QSet<QTreeWidgetItem*> dirtyLayerItems;

    //inner elements to update
    for(auto &key : elementIds) {
        auto item = this->_treeItemsByAtomId[key];
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

QTreeWidgetItem* MapLayoutTree::_getLayerItem(RPZAtom &atom) {
    return this->_getLayerItem(atom.metadata()->layer());
}

QTreeWidgetItem* MapLayoutTree::_getLayerItem(int layer) {
    
    auto layerElem = this->_layersItems[layer];
    
    if(!layerElem) {

        //define new
        layerElem = new LayerTreeItem();
        layerElem->setText(0, "Calque " + QString::number(layer));
        layerElem->setData(0, Qt::UserRole, QVariant(layer));
        layerElem->setIcon(0, QIcon(":/icons/app/manager/layer.png"));
        layerElem->setFlags(Qt::ItemIsEnabled);
        
        //add to layout
        this->_layersItems[layer] = layerElem;
        this->addTopLevelItem(layerElem);

        //initial sort
        this->setSortingEnabled(true);
        this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
        this->setSortingEnabled(false);
    }

    return layerElem;
}

QTreeWidgetItem* MapLayoutTree::_createTreeItem(RPZAtom &atom) {
    
    auto item = new QTreeWidgetItem();
    item->setText(0, atom.descriptor());
    item->setData(0, Qt::UserRole, atom.id());
    item->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled | 
            Qt::ItemNeverHasChildren | 
            Qt::ItemIsSelectable
        )
    );

    const auto type = atom.type();
    switch(type) {
        case AtomBase::Type::Drawing:
            item->setIcon(0, QIcon(":/icons/app/manager/drawing.png"));
            break;
    }

    //create or get the layer element
    auto layerElem = this->_getLayerItem(atom);
    layerElem->addChild(item);
    this->_updateLayerState(layerElem);

    return item;
}

void MapLayoutTree::_updateLayerState(QTreeWidgetItem* layerItem) {
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

void MapLayoutTree::keyPressEvent(QKeyEvent * event) {
    
    RPZTree::keyPressEvent(event);

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:

            //make sure there is a selection
            const auto selectedIds = this->_extractIdsFromSelection();
            if(!selectedIds.length()) return;

            emit elementsAlterationAsked(RPZAtom::Alteration::Removed, selectedIds);
            break;
    }

}

QVector<QUuid> MapLayoutTree::_extractIdsFromSelection() const {
    
    QVector<QUuid> idList;
    
    for(auto &i : this->selectedItems()) {
        const auto innerData = i->data(0, Qt::UserRole).toUuid();
        idList.append(innerData);
    }

    return idList;
}
