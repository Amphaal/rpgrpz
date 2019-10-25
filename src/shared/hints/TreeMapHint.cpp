#include "TreeMapHint.h"

TreeMapHint::TreeMapHint() : AlterationAcknoledger(Payload::Source::Local_MapLayout), 
    _layerIcon(new QIcon(QStringLiteral(u":/icons/app/manager/layer.png"))),
    _textIcon(new QIcon(QStringLiteral(u":/icons/app/tools/text.png"))),
    _drawingIcon(new QIcon(QStringLiteral(u":/icons/app/manager/drawing.png"))) { 

    //on rename
    QObject::connect(
        AssetsDatabase::get(), &AssetsDatabase::assetRenamed,
        this, &TreeMapHint::_onRenamedAsset
    );
    
    this->connectToAlterationEmissions();

}

void TreeMapHint::propagateFocus(const RPZAtomId &focusedRPZAtomId) {
    FocusedPayload payload(focusedRPZAtomId);
    AlterationHandler::get()->queueAlteration(this, payload);
}

void TreeMapHint::propagateSelection(const QVector<RPZAtomId> &selectedIds) {
    SelectedPayload payload(selectedIds);
    AlterationHandler::get()->queueAlteration(this, payload);
}

void TreeMapHint::_handleAlterationRequest(const AlterationPayload &payload) {

    auto type = payload.type();

    QList<QTreeWidgetItem*> out;
    LayerManipulationHelper mvHelper;

    //atom wielders format (eg INSERT / RESET)
    if(auto mPayload = dynamic_cast<const AtomsWielderPayload*>(&payload)) {
        
        {
            QMutexLocker l(&this->_m_layersItems);

            if(type == Payload::Alteration::Reset) {
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

                //if has assetHash, add it
                auto assetHash = atom.assetHash();
                if(!assetHash.isNull()) {
                    this->_RPZAtomIdsBoundByRPZAssetHash[assetHash].insert(atomId);
                }
            }
        }

        emit requestingUIAlteration(type, out);

    }

    //on remove
    else if(auto mPayload = dynamic_cast<const RemovedPayload*>(&payload)) {

        for (auto &id : mPayload->targetRPZAtomIds()) {
            
            //remove from internal list
            auto item = this->_atomTreeItemsById.take(id);
            out += item;

            //if has assetHash, remove it from tracking list
            auto tbrAtom_assetHash = RPZQVariant::assetHash(item);
            if(!tbrAtom_assetHash.isNull()) {
                this->_RPZAtomIdsBoundByRPZAssetHash[tbrAtom_assetHash].remove(id);
            }

            //
            mvHelper.toRemoveChildrenCountByLayerItem[item->parent()]++;

        }

        emit requestingUIAlteration(type, out);

    }

    //on metadata change
    else if(auto mPayload = dynamic_cast<const MetadataChangedPayload*>(&payload)) {
        
        auto updates = mPayload->updates();
        
        for (auto &id : mPayload->targetRPZAtomIds()) {
            auto item = this->_atomTreeItemsById.value(id);
            out += item;
            this->_handleItemMove(item, updates, mvHelper);
        }

        emit requestingUIUpdate(out, updates);
    }


    //on metadata change (bulk)
    else if(auto mPayload = dynamic_cast<const BulkMetadataChangedPayload*>(&payload)) {
        
        QHash<QTreeWidgetItem*, AtomUpdates> toUpdate;
        auto updatesById = mPayload->atomsUpdates();
        
        for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
            
            auto item = this->_atomTreeItemsById.value(i.key());
            auto updates = i.value();

            toUpdate.insert(item, updates);
            this->_handleItemMove(item, updates, mvHelper);
        }

        emit requestingUIUpdate(toUpdate);
    }

    //anything else
    else if(auto mPayload = dynamic_cast<const MultipleAtomTargetsPayload*>(&payload)) {
        
        for (auto &id : mPayload->targetRPZAtomIds()) {
            auto item = this->_atomTreeItemsById.value(id);
            out += item;
        }

        emit requestingUIAlteration(type, out);
    }

    //apply move if needed
    this->_applyMove(mvHelper);
    
}

void TreeMapHint::_handleItemMove(QTreeWidgetItem* toUpdate, const AtomUpdates &updatesMightContainMove, LayerManipulationHelper &mvHelper) {
    
    if(!updatesMightContainMove.contains(AtomParameter::Layer)) return;
    
    auto layerItem = toUpdate->parent();
    auto currentLayer =  RPZQVariant::atomLayer(layerItem);
    auto requestedLayer = updatesMightContainMove.value(AtomParameter::Layer).toInt();
    
    if(currentLayer == requestedLayer) return;

    //update move helper
    mvHelper.childrenMovedToLayer[requestedLayer].append(toUpdate);
    mvHelper.toRemoveChildrenCountByLayerItem[layerItem]++;

}

void TreeMapHint::_applyMove(LayerManipulationHelper &mvHelper) {

    //if move requested...
    if(mvHelper.childrenMovedToLayer.count()) {
        
        //may create new layer item
        for(auto layer : mvHelper.childrenMovedToLayer.keys()) {
            this->_mayCreateLayerItem(layer);
        }

        //ask for move
        emit requestingUIMove(mvHelper.childrenMovedToLayer);

    }

    //check to-delete layer items
    if(mvHelper.toRemoveChildrenCountByLayerItem.count()) {
        
        QList<QTreeWidgetItem*> mightDelete;

        for(auto i = mvHelper.toRemoveChildrenCountByLayerItem.begin(); i != mvHelper.toRemoveChildrenCountByLayerItem.end(); i++) {
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
                    auto layer = RPZQVariant::atomLayer(toDelete);
                    this->_layersItems.remove(layer);
                }
            }

            emit requestingUIAlteration(Payload::Alteration::Removed, mightDelete);
        }

    }
}


void TreeMapHint::_onRenamedAsset(const QString &assetHash, const QString &newName) {
    if(!this->_RPZAtomIdsBoundByRPZAssetHash.contains(assetHash)) return;

    QList<QTreeWidgetItem*> toUpdate;
    AtomUpdates updates {{ AtomParameter::AssetName, newName }};

    for(auto &RPZAtomId : this->_RPZAtomIdsBoundByRPZAssetHash.value(assetHash)) {
        toUpdate += this->_atomTreeItemsById.value(RPZAtomId);
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
    layerElem->setText(0, tr("Layer %1").arg(layer));
    RPZQVariant::setAtomLayer(layerElem, layer);
    layerElem->setIcon(0, *this->_layerIcon);
    layerElem->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled 
        )
    );
        
    //add to layout
    this->_layersItems.insert(layer, layerElem);
    
}

QTreeWidgetItem* TreeMapHint::_createTreeItem(const RPZAtom &atom) {
    
    auto item = new QTreeWidgetItem;
    item->setTextAlignment(1, Qt::AlignRight);
    item->setTextAlignment(2, Qt::AlignRight);
    
    const auto layer = atom.layer();
    const auto type = atom.type();

    //default flags
    item->setFlags(
        QFlags<Qt::ItemFlag>(
            Qt::ItemIsEnabled | 
            Qt::ItemNeverHasChildren |
            Qt::ItemIsSelectable
        )
    );

    item->setText(0, atom.toString());
    RPZQVariant::setAtomType(item, atom.type());
    RPZQVariant::setAtomId(item, atom.id());
    RPZQVariant::setAssetHash(item, atom.assetHash());
    RPZQVariant::setAtomLayer(item, layer);
    RPZQVariant::setAtomVisibility(item, atom.isHidden());
    RPZQVariant::setAtomAvailability(item, atom.isLocked());

    switch(type) {
        case RPZAtomType::Drawing:
            item->setIcon(0, *this->_drawingIcon);
            break;
        case RPZAtomType::Text:
            item->setIcon(0, *this->_textIcon);
            break;
        default:
            break;
    }

    this->_mayCreateLayerItem(layer);

    return item;
}
