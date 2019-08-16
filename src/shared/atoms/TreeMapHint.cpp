#include "TreeMapHint.h"

TreeMapHint::TreeMapHint(AtomsStorage* mapMaster) : AlterationAcknoledger(AlterationPayload::Source::Local_MapLayout), 
    AtomsContextualMenuHandler(mapMaster), 
    _layerIcon(new QIcon(":/icons/app/manager/layer.png")),
    _textIcon(new QIcon(":/icons/app/tools/text.png")),
    _drawingIcon(new QIcon(":/icons/app/manager/drawing.png")) { 

    //on rename
    QObject::connect(
        AssetsDatabase::get(), &AssetsDatabase::assetRenamed,
        this, &TreeMapHint::_onRenamedAsset
    );

}

void TreeMapHint::propagateFocus(snowflake_uid focusedAtomId) {
    FocusedPayload payload(focusedAtomId);
    AlterationHandler::get()->queueAlteration(this, payload);
}

void TreeMapHint::propagateSelection(QVector<snowflake_uid> &selectedIds) {
    SelectedPayload payload(selectedIds);
    AlterationHandler::get()->queueAlteration(this, payload);
}

void TreeMapHint::_handleAlterationRequest(AlterationPayload &payload) {

    auto type = payload.type();

    //selected...
    if(type == PayloadAlteration::PA_Selected) emit requestingItemSelectionClearing();
    if(type == PayloadAlteration::PA_Reset) {
        this->_atomTreeItemsById.clear();
        this->_layersItems.clear();
        this->_atomIdsBoundByRPZAssetHash.clear();
        emit requestingItemClearing();
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

}

RPZAtom* TreeMapHint::_handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) {
    
    QTreeWidgetItem* item = item = this->_atomTreeItemsById[targetedAtomId];

    switch(type) {

        case PayloadAlteration::PA_Reset:
        case PayloadAlteration::PA_Added: {
            
            auto atom = RPZAtom(alteration.toHash());
            
            item = this->_createTreeItem(atom);
            this->_atomTreeItemsById.insert(targetedAtomId, item);

            //if has assetId, add it
            auto assetId = atom.assetId();
            if(!assetId.isNull()) {
                this->_atomIdsBoundByRPZAssetHash[assetId].insert(targetedAtomId);
            }
        }
        break;

        case PayloadAlteration::PA_OwnerChanged: {
            auto user = RPZUser(alteration.toHash());
            this->_bindOwnerToItem(item, user);
        }
        break;

        case PayloadAlteration::PA_Removed: {
            
            RPZAssetHash tbrAtom_assetId = item->data(0, RPZUserRoles::AssetHash).toString();

            //if has assetId, remove it from tracking list
            if(!tbrAtom_assetId.isNull()) {
                    this->_atomIdsBoundByRPZAssetHash[tbrAtom_assetId].remove(targetedAtomId);
            }

            this->_atomTreeItemsById.remove(targetedAtomId);
            emit requestingItemDeletion(item);
        }
        break;

        case PayloadAlteration::PA_Selected: {
            emit requestingItemSelection(item);
        }
        break;

        case PayloadAlteration::PA_MetadataChanged:
        case PayloadAlteration::PA_BulkMetadataChanged: {
            auto partial = type == PayloadAlteration::PA_BulkMetadataChanged ? 
                                                        RPZAtom(alteration.toHash()) : 
                                                        MetadataChangedPayload::fromArgs(alteration);
            
            for(auto param : partial.editedMetadata()) {
                
                switch(param) {
                
                    case AtomParameter::Locked: {
                        emit requestingItemDataUpdate(
                            item, 1, 
                            {{RPZUserRoles::AtomAvailability, partial.isLocked()}} 
                        );
                    }
                    break;

                    case AtomParameter::Hidden: {
                        emit requestingItemDataUpdate(
                            item, 1, 
                            {{RPZUserRoles::AtomVisibility, partial.isHidden()}} 
                        );
                    }
                    break;

                    case AtomParameter::Layer: {
                        auto item = this->_atomTreeItemsById[targetedAtomId];
                        if(item) {
                            auto oldLayerItem = item->parent();
                            auto newLayerItem = this->_getLayerItem(partial.layer());
                            emit requestingItemMove(oldLayerItem, newLayerItem, item);
                        }

                    }
                    break;

                    default:
                        break;
                }
                
            }
        }   
        break;

        default:
            break;
    }

    return nullptr;
}

void TreeMapHint::_onRenamedAsset(const QString &assetId, const QString &newName) {
    if(!this->_atomIdsBoundByRPZAssetHash.contains(assetId)) return;

    for(auto &atomId : this->_atomIdsBoundByRPZAssetHash[assetId]) {
        auto itemToChange = this->_atomTreeItemsById[atomId];
        emit requestingItemTextChange(itemToChange, newName);
    }
}

QTreeWidgetItem* TreeMapHint::_getLayerItem(int layer) {
    
    auto layerElem = this->_layersItems[layer];
	if (layerElem) return layerElem;

    //if undef, create new
    layerElem = new LayerTreeItem();
    layerElem->setText(0, "Calque " + QString::number(layer));
    layerElem->setData(0, RPZUserRoles::AtomLayer, QVariant(layer));
    layerElem->setIcon(0, *this->_layerIcon);
    layerElem->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled 
        )
    );
        
    //add to layout
    this->_layersItems[layer] = layerElem;
    emit requestingItemInsertion(layerElem, nullptr);
    
    return layerElem;
}

void TreeMapHint::_bindOwnerToItem(QTreeWidgetItem* item, RPZUser &owner) {
    QHash<int, QVariant> newData {
        { RPZUserRoles::OwnerId, owner.id() },
        { RPZUserRoles::UserColor, owner.color()},
        { Qt::ToolTipRole, owner.toString() }
    };
    emit requestingItemDataUpdate(item, 2, newData);
}

QTreeWidgetItem* TreeMapHint::_createTreeItem(RPZAtom &atom) {
    
    auto item = new QTreeWidgetItem();
    
    item->setText(0, atom.descriptor());
    item->setData(0, RPZUserRoles::AtomId, atom.id());
    item->setData(0, RPZUserRoles::AssetHash, atom.assetId());

    item->setData(1, RPZUserRoles::AtomVisibility, atom.isHidden());
    item->setData(1, RPZUserRoles::AtomAvailability, atom.isLocked());

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
            item->setIcon(0, *this->_drawingIcon);
            break;
        case AtomType::Text:
            item->setIcon(0, *this->_textIcon);
            break;
        default:
            break;
    }

    //create or get the layer element
    auto layerElem = this->_getLayerItem(atom.layer());
    emit requestingItemInsertion(item, layerElem);

    return item;
}

void TreeMapHint::removeLayerItem(int layer) {
    this->_layersItems.remove(layer);
}