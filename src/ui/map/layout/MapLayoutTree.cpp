#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(QWidget * parent) : RPZTree(parent) {
    
    this->setColumnCount(3);

    this->setHeaderHidden(true);

    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);

    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

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

    auto selectedIds = this->_selectedAtomIds();
    this->_changeLayer(selectedIds, targetLayer);

    //unilateral event, expect only outer calls
    this->_emitAlteration(LayerChangedPayload(selectedIds, targetLayer));
}


void MapLayoutTree::_onElementDoubleClicked(QTreeWidgetItem * item, int column) {
    this->_emitAlteration(
        FocusedPayload(this->_extractAtomIdFromItem(item))
    );
}

void MapLayoutTree::_onElementSelectionChanged() {
    this->_emitAlteration(
        SelectedPayload(this->_selectedAtomIds())
    );
}

void MapLayoutTree::alterTreeElements(const QVariantHash &payload) {

    //prevent circular payloads
    AlterationPayload aPayload(payload);
    auto type = aPayload.type();
    if(aPayload.source() == this->_source) return;

    //special handling
    if(type == AlterationPayload::Alteration::Selected) this->clearSelection();
    if(type == AlterationPayload::Alteration::Reset) {
        for(auto item : this->_treeItemsByAtomId) {
            delete item;
        }
        this->_treeItemsByAtomId.clear();
    }

    //specific bulk handling for UI optimizations
    if(type == AlterationPayload::Alteration::LayerChanged) {
        auto temp = LayerChangedPayload(payload);
        this->_changeLayer(temp.targetAtomIds(), temp.layer());
    }

    //conditionnal handling by alteration
    auto alterations = aPayload.alterationByAtomId();
    for (QVariantHash::iterator i = alterations.begin(); i != alterations.end(); ++i) {
        
        auto key = i.key();
        auto item = this->_treeItemsByAtomId[key];

        switch(type) {
            case AlterationPayload::Alteration::Removed: {
                
                auto layerItem = item->parent();
                auto old_assetId = item->data(0, 666).toString();
                auto old_id = item->data(0, Qt::UserRole).toUuid();

                //if has assetId, remove it from tracking list
                if(!old_assetId.isNull()) {
                     this->_atomIdsBoundByAssetId[old_assetId].remove(oldItem);
                }

                this->_treeItemsByAtomId.remove(key);
                delete oldItem;

                //also remove layer
                this->_updateLayerState(layerItem);


            }
            break;

            case AlterationPayload::Alteration::Selected: {
                item->setSelected(true);
            }
            break;

            case AlterationPayload::Alteration::Reset:
            case AlterationPayload::Alteration::Added: {
                auto atom = RPZAtom(i.value().toHash());
                auto item = this->_createTreeItem(atom);
                this->_treeItemsByAtomId.insert(key, item);

                //if has assetId, add it
                auto assetId = atom.metadata()->assetId();
                if(!assetId.isNull()) {
                    this->_atomIdsBoundByAssetId[assetId].insert(key);
                }
            }
            break;
        }

    }

}

void MapLayoutTree::_onRenamedAsset(const QString &assetId, const QString &newName) {
    if(!this->_atomIdsBoundByAssetId.contains(assetId)) return;

    for(auto &atomId : this->_atomIdsBoundByAssetId[assetId]) {
        this->_treeItemsByAtomId[atomId]->setText(0, newName);
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
    item->setText(1, atom.owner().toString());

    item->setData(0, Qt::UserRole, atom.id());
    item->setData(0, 666, atom.metadata()->assetId());
    
    item->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled | 
            Qt::ItemNeverHasChildren | 
            Qt::ItemIsSelectable
        )
    );

    const auto type = atom.type();
    switch(type) {
        case RPZAtom::Type::Drawing:
            item->setIcon(0, QIcon(":/icons/app/manager/drawing.png"));
            break;
    }

    //create or get the layer element
    auto layerElem = this->_getLayerItem(atom.metadata()->layer());
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

void MapLayoutTree::_emitAlteration(AlterationPayload &payload) {
    payload.changeSource(this->_source);
    emit elementsAlterationAsked(payload);
}

void MapLayoutTree::keyPressEvent(QKeyEvent * event) {
    
    RPZTree::keyPressEvent(event);

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:

            //make sure there is a selection
            const auto selectedIds = this->_selectedAtomIds();
            if(!selectedIds.length()) return;

            this->_emitAlteration(RemovedPayload(selectedIds));
            break;
    }

}

QUuid MapLayoutTree::_extractAtomIdFromItem(QTreeWidgetItem* item) const {
    return item->data(0, Qt::UserRole).toUuid();
}

QVector<QUuid> MapLayoutTree::_selectedAtomIds() const {
    QVector<QUuid> idList;
    for(auto i : this->selectedItems()) {
        idList.append(this->_extractAtomIdFromItem(i));
    }
    return idList;
}
