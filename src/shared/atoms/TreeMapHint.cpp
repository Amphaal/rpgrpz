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

    //selected...
    if(type == PayloadAlteration::PA_Reset) {
        this->_atomTreeItemsById.clear();
        this->_layersItems.clear();
        this->_RPZAtomIdsBoundByRPZAssetHash.clear();
    }

    this->_UIUpdatesBuffer.clear();

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
        
        auto ids = mPayload->targetRPZAtomIds();
        auto args =  mPayload->args();
        
        for (auto id : ids) {
            this->_handlePayloadInternal(type, id, args);
        }

    }

    //send UI events
    if(type == PA_MetadataChanged || type == PA_BulkMetadataChanged) {
        emit requestingUIUpdate(type, this->_UIUpdatesBuffer);
    } else {
        emit requestingUIAlteration(type, this->_UIUpdatesBuffer.keys());
    }

}

RPZAtom* TreeMapHint::_handlePayloadInternal(const PayloadAlteration &type, RPZAtomId targetedRPZAtomId, const QVariant &alteration) {

    QTreeWidgetItem* item = nullptr;
    AtomUpdates maybeNewData;

    switch(type) {

        case PayloadAlteration::PA_Reset:
        case PayloadAlteration::PA_Added: {
            
            auto atom = RPZAtom(alteration.toHash());
            
            item = this->_createTreeItem(atom);
            this->_atomTreeItemsById.insert(targetedRPZAtomId, item);

            //if has assetId, add it
            auto assetId = atom.assetId();
            if(!assetId.isNull()) {
                this->_RPZAtomIdsBoundByRPZAssetHash[assetId].insert(targetedRPZAtomId);
            }
        }
        break;

        case PayloadAlteration::PA_OwnerChanged: {
            auto user = RPZUser(alteration.toHash());
            this->updateOwnerFromItem(item, user);
        }
        break;

        case PayloadAlteration::PA_Removed: {
            
            RPZAssetHash tbrAtom_assetId = item->data(0, RPZUserRoles::AssetHash).toString();

            //if has assetId, remove it from tracking list
            if(!tbrAtom_assetId.isNull()) {
                    this->_RPZAtomIdsBoundByRPZAssetHash[tbrAtom_assetId].remove(targetedRPZAtomId);
            }

            item = this->_atomTreeItemsById.take(targetedRPZAtomId);
        }
        break;

        case PayloadAlteration::PA_MetadataChanged:
        case PayloadAlteration::PA_BulkMetadataChanged: {
            auto partial = type == PayloadAlteration::PA_BulkMetadataChanged ? 
                                                        RPZAtom(alteration.toHash()) : 
                                                        MetadataChangedPayload::fromArgs(alteration);
            
            for(auto param : partial.editedMetadata()) {
                maybeNewData.insert(param, partial.metadata(param));
            }
        }   
        break;

        default:
            break;
    }

    //get graphics item as default
    if(!item) item = this->_atomTreeItemsById[targetedRPZAtomId];

    //update buffers for UI event emission
    this->_UIUpdatesBuffer.insert(item, maybeNewData);

    return nullptr;
}

void TreeMapHint::_onRenamedAsset(const QString &assetId, const QString &newName) {
    if(!this->_RPZAtomIdsBoundByRPZAssetHash.contains(assetId)) return;

    QHash<QTreeWidgetItem*, AtomUpdates> toUpdate;
    AtomUpdates out {{ AssetName, newName }};

    for(auto &RPZAtomId : this->_RPZAtomIdsBoundByRPZAssetHash[assetId]) {
        auto itemToChange = this->_atomTreeItemsById[RPZAtomId];
        toUpdate.insert(itemToChange, out);
    }

    emit requestingUIUpdate(PA_MetadataChanged, toUpdate);
}

QTreeWidgetItem* TreeMapHint::_getLayerItem(int layer) {
    
    auto layerElem = this->_layersItems.value(layer);
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
    this->_layersItems.insert(layer, layerElem);
    
    return layerElem;
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

QTreeWidgetItem* TreeMapHint::_createTreeItem(RPZAtom &atom) {
    
    auto item = new QTreeWidgetItem();
    
    item->setText(0, atom.descriptor());
    item->setData(0, RPZUserRoles::AtomId, atom.id());
    item->setData(0, RPZUserRoles::AssetHash, atom.assetId());

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

    const auto type = atom.type();
    const auto layer = atom.layer();

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
    auto layerElem = this->_getLayerItem(layer);
    layerElem->addChild(layerElem);

    return item;
}

void TreeMapHint::removeLayerItem(int layer) {
    this->_layersItems.remove(layer);
}