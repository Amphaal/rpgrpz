#include "MapHint.h"

MapHint::MapHint() {};

QVector<RPZAsset> MapHint::fetchHistory() {
    return this->_assetsById.values().toVector();
}

//handle network and local evts emission
void MapHint::_emitAlteration(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements) {

    emit assetsAlteredForLocal(state, elements);

    if(RPZAsset::networkAlterations.contains(state) && !this->_preventNetworkAlterationEmission) {
        emit assetsAlteredForNetwork(state, elements);
    }

} 

QVariantHash MapHint::packageForNetworkSend(const RPZAsset::RPZAsset::Alteration &state, QVector<RPZAsset> &assets) {
    AlterationPayload payload(state, assets);
    return payload.toVariantHash();
}


/////////////////
/* END NETWORK */
/////////////////

//////////////
/* ELEMENTS */
//////////////

//register actions
QUuid MapHint::_alterSceneInternal(const RPZAsset::Alteration &alteration, RPZAsset &asset) {

    //get the Uuids
    auto elemId = asset.id();
    auto ownerId = asset.owner().id();
    
    switch(alteration) {

        //on addition
        case RPZAsset::Alteration::Reset:
        case RPZAsset::Alteration::Added:
            
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
        case RPZAsset::Alteration::Removed:

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
void MapHint::_alterSceneGlobal(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) { 

    if(alteration == RPZAsset::Alteration::Reset) {
        this->_selfElements.clear();
        this->_assetsById.clear();
        this->_foreignElementIdsByOwnerId.clear();
    }

    //handling
    for(auto &asset : assets) {
        this->_alterSceneInternal(alteration, asset);
    }
    //emit event
    this->_emitAlteration(alteration, assets);
}


//helper
void MapHint::alterSceneFromAsset(const RPZAsset::Alteration &alteration, RPZAsset &asset) {
    QVector<RPZAsset> list;
    list.append(asset);
    return this->_alterSceneGlobal(alteration, list);
}


//helper
void MapHint::alterSceneFromAssets(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) {
    return this->_alterSceneGlobal(alteration, assets);
}

//helper
void MapHint::alterSceneFromIds(const RPZAsset::Alteration &alteration, const QVector<QUuid> &elementIds) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elementIds));
}


//helper
QVector<RPZAsset> MapHint::_fetchAssets(const QVector<QUuid> &listToFetch) const {
   QVector<RPZAsset> list;

    for(auto &e : listToFetch) {
        list.append(this->_assetsById[e]);
    }

   return list; 
}


//////////////////
/* END ELEMENTS */
//////////////////
