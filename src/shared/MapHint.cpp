#include "MapHint.h"

MapHint::MapHint() {};

QList<Asset> MapHint::fetchHistory() {
    return this->_assetsById.values();
}

//handle network and local evts emission
void MapHint::_emitAlteration(const Alteration &state, QList<Asset> &elements) {

    emit assetsAlteredForLocal(state, elements);

    if(this->networkAlterations.contains(state)) {
        emit assetsAlteredForNetwork(state, elements);
    }

} 

QVariantList MapHint::packageForNetworkSend(const MapHint::Alteration &state, QList<Asset> &assets) {
    
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

QUuid MapHint::_defineId(const Alteration &alteration, Asset &asset) {
    auto elemId = asset.id();
    if(elemId.isNull() && alteration == Alteration::Added) {
        elemId = QUuid::createUuid();
        asset.setId(elemId);
    }
    return elemId;
}

//register actions
QUuid MapHint::_alterSceneInternal(const Alteration &alteration, Asset &asset) {

    //get the Uuids
    auto elemId = this->_defineId(alteration, asset);
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
            if(!this->_assetsById.contains(elemId)) {
                this->_assetsById.insert(elemId, asset);
            }
            break;
        
        //on removal
        case Alteration::Removed:

            //unbind from owners
            if(!ownerId.isNull()) {
               this->_foreignElementIdsByOwnerId[ownerId].remove(elemId);
            } else {
                this->_selfElements.remove(elemId);
            }

            //update 
            this->_assetsById.remove(elemId);
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
    this->_emitAlteration(alteration, assets);
}


//helper
void MapHint::alterSceneFromAsset(const Alteration &alteration, Asset &asset) {
    QList<Asset> list;
    list.append(asset);
    return this->_alterSceneGlobal(alteration, list);
}

//helper
void MapHint::alterSceneFromIds(const Alteration &alteration, const QList<QUuid> &elementIds) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elementIds));
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
