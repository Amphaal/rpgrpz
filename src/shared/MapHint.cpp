#include "MapHint.h"


//handle network and local evts emission
void MapHint::_emitAlteration(QList<Asset> &elements, const Alteration &state) {
    
    emit mapElementsAltered(elements, state);

    if(this->networkAlterations.contains(state)) {
        emit notifyNetwork_mapElementsAltered(elements, state);
    }

} 

QVariantList MapHint::packageForNetworkSend(QList<Asset> &assets, const MapHint::Alteration &state) {
    
    QVariantList toSend;

    //parse data
    QVariantHash data;
    data.insert("state", state);

    QVariantHash assetsContainer;
    for(auto &asset : assets) {

        //contains subdata
        QVariantHash binder;

        //in case when you need to bind raw data
        if(state == MapHint::Alteration::Added) {
            
            //raw data container
            QByteArray assetData = NULL;

            //switch on asset type
            switch(asset.type()) {
                
                //drawing...
                case AssetBase::Type::Drawing:
                    auto casted = (QGraphicsPathItem*)asset.graphicsItem();
                    const auto path = casted->path();
                    assetData = JSONSerializer::toBase64(path);
                    break;

            }

            binder.insert("data", assetData);
        }   

        //default data
        binder.insert("type", asset.type());
        binder.insert("owner", asset.ownerId());

        assetsContainer.insert(asset.id().toString(), binder);
    }

    data.insert("assets", assetsContainer);
    toSend.append(data);

    return toSend;
}


/////////////////
/* END NETWORK */
/////////////////

//////////////
/* ELEMENTS */
//////////////

//register actions
QUuid MapHint::_alterSceneInternal(const Alteration &alteration, Asset &asset) {

    //get the Uuids
    auto elemId = asset.id();
    if(elemId.isNull()) {
        elemId = alteration == Alteration::Added ? QUuid::createUuid() : this->_idsByGraphicItem[asset.graphicsItem()];
        asset.setId(elemId);
    }
    auto ownerId = asset.ownerId();
    
    switch(alteration) {
        
        //on addition
        case Alteration::Added:
            
            //bind to owners
            if(!ownerId.isNull()) {
                if(!this->_foreignElementIdsByOwnerId.contains(ownerId)) {
                    this->_foreignElementIdsByOwnerId.insert(ownerId, QSet<QUuid>());
                }
                this->_foreignElementIdsByOwnerId[ownerId].insert(elemId);
            } else {
                this->_selfElements.insert(elemId);
            }

            //bind elem
            if(!this->_assetsById.contains(ownerId)) {
                this->_assetsById.insert(elemId, asset);
                this->_idsByGraphicItem.insert(asset.graphicsItem(), elemId);
            }
            break;
        
        //on focus
        case Alteration::Focused:
            this->centerOn(asset.graphicsItem());
            break;

        //on selection
        case MapHint::Alteration::Selected:
            asset.graphicsItem()->setSelected(true);
            break;

        //on removal
        case Alteration::Removed:

            //unbind from owners
            if(!ownerId.isNull()) {
               this->_foreignElementIdsByOwnerId[ownerId].remove(elemId);
            } else {
                this->_selfElements.remove(elemId);
            }

            //remove from map
            delete asset.graphicsItem();

            //update 
            this->_assetsById.remove(elemId);
            this->_idsByGraphicItem.remove(asset.graphicsItem());
            break;

    }

    return elemId;
}

//alter Scene
void MapHint::_alterSceneGlobal(const Alteration &alteration, QList<Asset> &assets) { 
    
    //handling
    for(auto &asset : assets) {
        this->_alterSceneInternal(alteration, asset);
    }
    //emit event
    this->_emitAlteration(assets, alteration);
}

//helper
void MapHint::_alterScene(const Alteration &alteration, Asset &asset) {
    QList<Asset> list;
    list.append(asset);
    return this->_alterSceneGlobal(alteration, list);
}


//helper
void MapHint::_alterScene(const Alteration &alteration, const QList<QUuid> &elementIds) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elementIds));
}


//from external instructions
void MapHint::alterScene(const QList<QUuid> &elementIds, const MapHint::Alteration &state) {
    this->_alterScene(state, elementIds);
}



//helper
QList<Asset> MapHint::_fetchAssets(const QList<QUuid> &listToFetch) const {
   QList<Asset> list;

    for(auto &e : listToFetch) {
        list.append(this->_assetsById[e]);
    }

   return list; 
}


//////////////////
/* END ELEMENTS */
//////////////////
