#include "MapNavigator.h"


void MapNavigator::unpackFromNetworkReceived(const QVariantList &package) {

    //container
    QList<Asset> out; 

    //get data
    const auto data = package[0].toHash();
    const auto state = (MapHint::Alteration)data["state"].toInt();

    //iterate through assets
    const auto assetsContainer = data["assets"].toHash();
    for(auto &key : assetsContainer.keys()) {

        //get data
        const auto elemId = QUuid::fromString(key);
        const auto binder = assetsContainer[key].toHash();
        const auto binderType = (AssetBase::Type)binder["type"].toInt();
        const auto binderOwner = binder["owner"].toUuid();

        //build asset or fetch it
        Asset newAsset;

        //if new element from network
        if(state == MapHint::Alteration::Added) {
            
            //elem already exists, shouldnt rewrite it!
            if(this->_assetsById.contains(elemId)) continue;

            //newly created map elem
            QGraphicsItem * newItem;

            //depending on assetType...
            switch(binderType) {
                
                //drawing...
                case AssetBase::Type::Drawing:
                    const QPainterPath path = JSONSerializer::fromBase64(binder["data"]);
                    newItem = this->scene()->addPath(path);
                    break;
            
            }

            //define
            newAsset = Asset(binderType, newItem, key, binderOwner);

        } else {
            //fetch from stock
            newAsset = this->_assetsById[key];
        }
        
        //add it to container
        out.append(newAsset);
    }

    //process new state
    this->_alterSceneGlobal(state, out);
}

//helper
void MapNavigator::_alterScene(const Alteration &alteration, const QList<QGraphicsItem*> &elements) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elements));
}

//helper
QList<Asset> MapNavigator::_fetchAssets(const QList<QGraphicsItem*> &listToFetch) const {
    QList<Asset> list;

    for(auto &e : listToFetch) {
        list.append(this->_assetsById[this->_idsByGraphicItem[e]]);
    }

    return list;
}

//from external instructions
void MapNavigator::alterScene(const QList<QUuid> &elementIds, const MapHint::Alteration &state) {
    
    this->_externalInstructionPending = true;

    //update internal state
    MapHint::alterScene(elementIds, state);

    this->_externalInstructionPending = false;
}

//alter Scene
void MapNavigator::_alterSceneGlobal(const Alteration &alteration, QList<Asset> &assets) { 
    
    //make sure to clear selection before selecting new
    if(alteration == Alteration::Selected) this->scene->clearSelection();
    if(alteration == MapHint::Alteration::Removed) this->_deletionProcessing = true;

    MapHint::_alterSceneGlobal(alteration, assets);

    this->_deletionProcessing = false;

}