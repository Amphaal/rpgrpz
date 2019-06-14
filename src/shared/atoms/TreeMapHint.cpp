#include "TreeMapHint.h"

TreeMapHint::TreeMapHint(QTreeWidget* boundTree) : AtomsHandler(AlterationPayload::Source::Local_MapLayout), 
    AtomsContextualMenuHandler(this, boundTree), 
    _boundTree(boundTree) { 
    
    //selection changed
    QObject::connect(
        this->_boundTree, &QTreeWidget::itemSelectionChanged,
        this, &TreeMapHint::_onElementSelectionChanged
    );

    //focus
    QObject::connect(
        this->_boundTree, &QTreeWidget::itemDoubleClicked,
        this, &TreeMapHint::_onElementDoubleClicked
    );

       //on rename
    QObject::connect(
        AssetsDatabase::get(), &AssetsDatabase::assetRenamed,
        this, &TreeMapHint::_onRenamedAsset
    );

}


void TreeMapHint::_onElementDoubleClicked(QTreeWidgetItem * item, int column) {
    auto focusedAtomId = this->_extractAtomIdFromItem(item);
    if(!focusedAtomId) return;

    auto payload = FocusedPayload(focusedAtomId);
    this->_emitAlteration(payload);
}

void TreeMapHint::_onElementSelectionChanged() {
    
    auto selected = this->_selectedAtomIds();
    if(!selected.count()) this->_boundTree->clearFocus();

    if(this->_preventInnerGIEventsHandling) return;

    auto payload = SelectedPayload(selected);
    this->_emitAlteration(payload);
}

void TreeMapHint::_handlePayload(AlterationPayload &payload) {

    //prevent circular payloads
    if(payload.source() == this->_source) {
        this->_preventInnerGIEventsHandling = false;
        return;
    }
    
    this->_preventInnerGIEventsHandling = true;

    auto type = payload.type();

    //selected...
    if(type == PayloadAlteration::Selected) this->_boundTree->clearSelection();
    if(type == PayloadAlteration::Reset) {

        for(auto item : this->_atomTreeItemsById) delete item;
        this->_atomTreeItemsById.clear();

        for(auto layerItem : this->_layersItems) delete layerItem;
        this->_layersItems.clear();
        
        this->_atomIdsBoundByAssetId.clear();
    }

    //atom wielders format
    if(auto bPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        
        auto atoms  = bPayload->atoms();
        
        for (RPZMap<RPZAtom>::iterator i = atoms.begin(); i != atoms.end(); ++i) {
            
            auto snowflakeId = i.key();
            auto atom = i.value();

            this->_handlePayloadInternal(type, snowflakeId, atom);
        }

    }

    //multi target format
    if(auto mPayload = dynamic_cast<MultipleTargetsPayload*>(&payload)) {
        
        auto ids = mPayload->targetAtomIds();
        auto args =  mPayload->args();
        
        for (auto id : ids) {
            this->_handlePayloadInternal(type, id, args);
        }

    }

    this->_preventInnerGIEventsHandling = false;
    
    this->_emitAlteration(payload);
}

RPZAtom* TreeMapHint::_handlePayloadInternal(const PayloadAlteration &type, const snowflake_uid &targetedAtomId, const QVariant &alteration) {
    
    auto item = this->_atomTreeItemsById[targetedAtomId];

    switch(type) {

        case PayloadAlteration::Reset:
        case PayloadAlteration::Added: {
            
            auto atom = RPZAtom(alteration.toHash());
            
            auto item = this->_createTreeItem(atom);
            this->_atomTreeItemsById.insert(targetedAtomId, item);

            //if has assetId, add it
            auto assetId = atom.assetId();
            if(!assetId.isNull()) {
                this->_atomIdsBoundByAssetId[assetId].insert(targetedAtomId);
            }
        }
        break;

        case PayloadAlteration::OwnerChanged: {
            this->_bindOwnerToItem(item, RPZUser(alteration.toHash()));
        }
        break;

        case PayloadAlteration::Removed: {
            
            auto layerItem = item->parent();
            auto tbrAtom_assetId = item->data(0, LayoutCustomRoles::AssetIdRole).toString();

            //if has assetId, remove it from tracking list
            if(!tbrAtom_assetId.isNull()) {
                    this->_atomIdsBoundByAssetId[tbrAtom_assetId].remove(targetedAtomId);
            }

            this->_atomTreeItemsById.remove(targetedAtomId);
            delete item;

            //also remove layer
            this->_updateLayerState(layerItem);
        }
        break;

        case PayloadAlteration::Selected: {
            item->setSelected(true);
        }
        break;

        case PayloadAlteration::MetadataChanged:
        case PayloadAlteration::BulkMetadataChanged: {
            auto partial = type == PayloadAlteration::BulkMetadataChanged ? RPZAtom(alteration.toHash()) : MetadataChangedPayload::fromArgs(alteration);
            
            for(auto param : partial.orderedEditedMetadata()) {
                
                switch(param) {
                
                    case AtomParameter::Locked: {
                        auto isLocked = partial.isLocked();
                        item->setData(1, LayoutCustomRoles::AvailabilityRole, isLocked);
                    }
                    break;

                    case AtomParameter::Hidden: {
                        auto isHidden = partial.isHidden();
                        item->setData(1, LayoutCustomRoles::VisibilityRole, isHidden);
                    }
                    break;

                    case AtomParameter::Layer: {
                        QVector<snowflake_uid> list {targetedAtomId};
                        this->_changeLayer(list, partial.layer());
                    }
                }
                
            }
        }   
        break;
    }

    return nullptr;
}

void TreeMapHint::_onRenamedAsset(const QString &assetId, const QString &newName) {
    if(!this->_atomIdsBoundByAssetId.contains(assetId)) return;

    for(auto &atomId : this->_atomIdsBoundByAssetId[assetId]) {
        this->_atomTreeItemsById[atomId]->setText(0, newName);
    }
}

void TreeMapHint::_changeLayer(QVector<snowflake_uid> &elementIds, int newLayer) {

    QSet<QTreeWidgetItem*> dirtyLayerItems;

    //inner elements to update
    for(auto &key : elementIds) {
        auto item = this->_atomTreeItemsById[key];
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

QTreeWidgetItem* TreeMapHint::_getLayerItem(int layer) {
    
    auto layerElem = this->_layersItems[layer];
    
    if(!layerElem) {

        //define new
        layerElem = new LayerTreeItem();
        layerElem->setText(0, "Calque " + QString::number(layer));
        layerElem->setData(0, Qt::UserRole, QVariant(layer));
        layerElem->setIcon(0, QIcon(":/icons/app/manager/layer.png"));
        layerElem->setFlags(
            QFlags<Qt::ItemFlag>(
                Qt::ItemIsEnabled 
            )
        );
        
        //add to layout
        this->_layersItems[layer] = layerElem;
        this->_boundTree->addTopLevelItem(layerElem);

        //initial sort
        this->_boundTree->setSortingEnabled(true);
        this->_boundTree->sortByColumn(0, Qt::SortOrder::DescendingOrder);
        this->_boundTree->setSortingEnabled(false);
    }

    return layerElem;
}

void TreeMapHint::_bindOwnerToItem(QTreeWidgetItem* item, RPZUser &owner) {
    item->setData(2, LayoutCustomRoles::OwnerIdRole, owner.id());
    item->setData(2, Qt::UserRole, owner.color());
    item->setData(2, Qt::ToolTipRole, owner.toString());
}

QTreeWidgetItem* TreeMapHint::_createTreeItem(RPZAtom &atom) {
    
    auto item = new QTreeWidgetItem();
    
    item->setText(0, atom.descriptor());
    item->setData(0, Qt::UserRole, atom.id());
    item->setData(0, LayoutCustomRoles::AssetIdRole, atom.assetId());

    item->setData(1, LayoutCustomRoles::VisibilityRole, atom.isHidden());
    item->setData(1, LayoutCustomRoles::AvailabilityRole, atom.isLocked());

    auto owner = atom.owner();
    this->_bindOwnerToItem(item, owner);

    item->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled | 
            Qt::ItemNeverHasChildren | 
            Qt::ItemIsSelectable
        )
    );

    const auto type = atom.type();
    switch(type) {
        case AtomType::Drawing:
            item->setIcon(0, QIcon(":/icons/app/manager/drawing.png"));
            break;
        case AtomType::Text:
            item->setIcon(0, QIcon(":/icons/app/tools/text.png"));
            break;
    }

    //create or get the layer element
    auto layerElem = this->_getLayerItem(atom.layer());
    layerElem->addChild(item);
    this->_updateLayerState(layerElem);

    return item;
}

void TreeMapHint::_updateLayerState(QTreeWidgetItem* layerItem) {
    if(layerItem->childCount()) {
        //has children, update count column
        layerItem->setText(2, QString::number(layerItem->childCount()));
        layerItem->setExpanded(true);
    } else {
        //has no more children, remove
        auto layer = layerItem->data(0, Qt::UserRole).toInt();
        delete this->_layersItems.take(layer);
    }
}


snowflake_uid TreeMapHint::_extractAtomIdFromItem(QTreeWidgetItem* item) const {
    return item->data(0, Qt::UserRole).toULongLong();
}

QVector<snowflake_uid> TreeMapHint::_selectedAtomIds() {
    QVector<snowflake_uid> idList;
    for(auto i : this->_boundTree->selectedItems()) {
        auto boundId = this->_extractAtomIdFromItem(i);
        if(boundId) idList.append(boundId);
    }
    return idList;
}
