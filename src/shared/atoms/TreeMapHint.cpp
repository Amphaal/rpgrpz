#include "TreeMapHint.h"

TreeMapHint::TreeMapHint() : AlterationAcknoledger(AlterationPayload::Source::Local_MapLayout), 
    _layerIcon(new QIcon(":/icons/app/manager/layer.png")),
    _textIcon(new QIcon(":/icons/app/tools/text.png")),
    _drawingIcon(new QIcon(":/icons/app/manager/drawing.png")) { 

    //on rename
    QObject::connect(
        AssetsDatabase::get(), &AssetsDatabase::assetRenamed,
        this, &TreeMapHint::_onRenamedAsset
    );
    
    this->connectToAlterationEmissions();

}

void TreeMapHint::propagateFocus(RPZAtomId focusedRPZAtomId) {
    FocusedPayload payload(focusedRPZAtomId);
    AlterationHandler::get()->queueAlteration(this, payload);
}

void TreeMapHint::propagateSelection(QVector<RPZAtomId> &selectedIds) {
    SelectedPayload payload(selectedIds);
    AlterationHandler::get()->queueAlteration(this, payload);
}

void TreeMapHint::_handleAlterationRequest(AlterationPayload &payload) {

    auto type = payload.type();

    QList<QTreeWidgetItem*> out;
    this->_mvHelper = LayerManipulationHelper();

    //atom wielders format (eg INSERT / RESET)
    if(auto mPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        
        {
            QMutexLocker l(&this->_m_layersItems);

            if(type == PayloadAlteration::PA_Reset) {
                this->_layersItems.clear();
                this->_atomTreeItemsById.clear();
                this->_RPZAtomIdsBoundByRPZAssetHash.clear();
            }

            auto atoms  = mPayload->atoms();
            for (auto i = atoms.begin(); i != atoms.end(); ++i) {
                
                auto atomId = i.key();
                auto atom = i.value();

                auto newItem = this->_createTreeItem(atom);
                out += newItem;
                this->_atomTreeItemsById.insert(atomId, newItem);

                //if has assetId, add it
                auto assetId = atom.assetId();
                if(!assetId.isNull()) {
                    this->_RPZAtomIdsBoundByRPZAssetHash[assetId].insert(atomId);
                }
            }
        }

        emit requestingUIAlteration(type, out);

    }

    //on owner changing
    else if(auto mPayload = dynamic_cast<OwnerChangedPayload*>(&payload)) {

        for (auto &id : mPayload->targetRPZAtomIds()) {
            out += this->_atomTreeItemsById[id];
        }

        emit requestingUIUserChange(out, mPayload->newOwner());
    
    }

    //on remove
    else if(auto mPayload = dynamic_cast<RemovedPayload*>(&payload)) {

        for (auto &id : mPayload->targetRPZAtomIds()) {
            
            //remove from internal list
            auto item = this->_atomTreeItemsById.take(id);
            out += item;

            RPZAssetHash tbrAtom_assetId = item->data(0, RPZUserRoles::AssetHash).toString();

            //if has assetId, remove it from tracking list
            if(!tbrAtom_assetId.isNull()) {
                this->_RPZAtomIdsBoundByRPZAssetHash[tbrAtom_assetId].remove(id);
            }

            //
            this->_mvHelper.toRemoveChildrenCountByLayerItem[item->parent()]++;

        }

        emit requestingUIAlteration(type, out);

    }

    //on metadata change
    else if(auto mPayload = dynamic_cast<MetadataChangedPayload*>(&payload)) {
        
        auto updates = mPayload->updates();
        
        for (auto &id : mPayload->targetRPZAtomIds()) {
            auto item = this->_atomTreeItemsById[id];
            out += item;
            this->_handleItemMove(item, updates);
        }

        emit requestingUIUpdate(out, updates);
    }


    //on metadata change (bulk)
    else if(auto mPayload = dynamic_cast<BulkMetadataChangedPayload*>(&payload)) {
        
        QHash<QTreeWidgetItem*, AtomUpdates> toUpdate;
        auto updatesById = mPayload->atomsUpdates();
        
        for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
            
            auto item = this->_atomTreeItemsById[i.key()];
            auto updates = i.value();

            toUpdate.insert(item, updates);
            this->_handleItemMove(item, updates);
        }

        emit requestingUIUpdate(toUpdate);
    }

    //anything else
    else if(auto mPayload = dynamic_cast<MultipleTargetsPayload*>(&payload)) {
        
        for (auto &id : mPayload->targetRPZAtomIds()) {
            auto item = this->_atomTreeItemsById[id];
            out += item;
        }

        emit requestingUIAlteration(type, out);
    }

    //request a move if needed
    if(this->_mvHelper.childrenMovedToLayer.count()) {
        emit requestingUIMove(this->_mvHelper.childrenMovedToLayer);
    }

    //check to-delete layer items
    if(this->_mvHelper.toRemoveChildrenCountByLayerItem.count()) {
        
        QList<QTreeWidgetItem*> mightDelete;

        for(auto i = this->_mvHelper.toRemoveChildrenCountByLayerItem.begin(); i != this->_mvHelper.toRemoveChildrenCountByLayerItem.end(); i++) {
            auto layerItemMaybeToRemove = i.key();
            if(layerItemMaybeToRemove->childCount() <= i.value()) {
                mightDelete += layerItemMaybeToRemove;
            }
        }

        //update layers items
        if(mightDelete.count()) {

            {
                QMutexLocker l(&this->_m_layersItems);
                for(auto toDelete : mightDelete) {
                    auto layer = toDelete->data(0, RPZUserRoles::AtomLayer).toInt();
                    this->_layersItems.remove(layer);
                }
            }

            emit requestingUIAlteration(PA_Removed, mightDelete);
        }

    }
    
}

void TreeMapHint::_handleItemMove(QTreeWidgetItem* toUpdate, const AtomUpdates &updatesMightContainMove) {
    
    if(!updatesMightContainMove.contains(Layer)) return;
    
    auto layerItem = toUpdate->parent();
    auto currentLayer = layerItem->data(0, RPZUserRoles::AtomLayer).toInt();
    auto requestedLayer = updatesMightContainMove[Layer].toInt();
    
    if(currentLayer == requestedLayer) return;

    this->_mvHelper.childrenMovedToLayer[requestedLayer].append(toUpdate);
    this->_mvHelper.toRemoveChildrenCountByLayerItem[layerItem]++;

}


void TreeMapHint::_onRenamedAsset(const QString &assetId, const QString &newName) {
    if(!this->_RPZAtomIdsBoundByRPZAssetHash.contains(assetId)) return;

    QList<QTreeWidgetItem*> toUpdate;
    AtomUpdates updates {{ AssetName, newName }};

    for(auto &RPZAtomId : this->_RPZAtomIdsBoundByRPZAssetHash[assetId]) {
        toUpdate += this->_atomTreeItemsById[RPZAtomId];
    }

    emit requestingUIUpdate(toUpdate, updates);
}

QTreeWidgetItem* TreeMapHint::getLayerItem(int layer) const {
    QMutexLocker l(&this->_m_layersItems);
    return this->_layersItems.value(layer);
}

void TreeMapHint::_mayCreateLayerItem(int layer) {

    auto layerElem = this->_layersItems.value(layer);
	if (layerElem) return;

    //if undef, create new
    layerElem = new LayerTreeItem();
    layerElem->setTextAlignment(1, Qt::AlignRight);
    layerElem->setTextAlignment(2, Qt::AlignRight);
    layerElem->setText(0, "Calque " + QString::number(layer));
    layerElem->setData(0, RPZUserRoles::AtomLayer, layer);
    layerElem->setIcon(0, *this->_layerIcon);
    layerElem->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled 
        )
    );
        
    //add to layout
    this->_layersItems.insert(layer, layerElem);
}

void TreeMapHint::updateOwnerFromItem(QTreeWidgetItem* item, const RPZUser &owner) {
    
    QHash<int, QVariant> newData {
        { RPZUserRoles::OwnerId, owner.id() },
        { RPZUserRoles::UserColor, owner.color()},
        { Qt::ToolTipRole, owner.toString() }
    };

    item->setData(2, RPZUserRoles::OwnerId, owner.id());
    item->setData(2, RPZUserRoles::UserColor, owner.color());
    item->setData(2, Qt::ToolTipRole, owner.toString());
}

QTreeWidgetItem* TreeMapHint::_createTreeItem(const RPZAtom &atom) {
    
    auto item = new QTreeWidgetItem();
    item->setTextAlignment(1, Qt::AlignRight);
    item->setTextAlignment(2, Qt::AlignRight);
    
    const auto layer = atom.layer();
    const auto type = atom.type();

    item->setText(0, atom.descriptor());
    item->setData(0, RPZUserRoles::AtomId, atom.id());
    item->setData(0, RPZUserRoles::AssetHash, atom.assetId());
    item->setData(0, RPZUserRoles::AtomLayer, layer);

    item->setData(1, RPZUserRoles::AtomVisibility, atom.isHidden());
    item->setData(1, RPZUserRoles::AtomAvailability, atom.isLocked());

    auto owner = atom.owner();
    this->updateOwnerFromItem(item, owner);

    item->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled | 
            Qt::ItemNeverHasChildren | 
            Qt::ItemIsSelectable
        )
    );

    switch(type) {
        case AtomType::Drawing:
            item->setIcon(0, *this->_drawingIcon);
            break;
        case AtomType::Text:
            item->setIcon(0, *this->_textIcon);
            break;
        default:
            break;
    }

    this->_mayCreateLayerItem(layer);

    return item;
}