#include "MapNavigator.h"

MapNavigator::MapNavigator(QWidget *parent) : QGraphicsView(parent), _hints(new MapHint) {
    
    //on selection
    QObject::connect(
        this->scene(), &QGraphicsScene::selectionChanged,
        this, &MapNavigator::_onSceneSelectionChanged
    );

    //to route from MapHints
    QObject::connect(
        this->_hints, &MapHint::mapElementsAltered,
        [&](QList<Asset> &elements, const MapHint::Alteration &state) {
            emit mapElementsAltered(elements, state);
        }
    );

    //to route from MapHints
    QObject::connect(
        this->_hints, &MapHint::notifyNetwork_mapElementsAltered,
        [&](QList<Asset> &elements, const MapHint::Alteration &state) {
            emit notifyNetwork_mapElementsAltered(elements, state);
        }
    );
};


void MapNavigator::_onSceneSelectionChanged() {

    if(this->_externalInstructionPending || this->_deletionProcessing) return;

    //emit event
    auto mapToEvt = this->_fetchAssets(this->scene()->selectedItems());
    this->_hints->_emitAlteration(mapToEvt, MapHint::Alteration::Selected);

}

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
            if(this->_hints->_assetsById.contains(elemId)) continue;

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
            newAsset = this->_hints->_assetsById[key];
        }
        
        //add it to container
        out.append(newAsset);
    }

    //process new state
    this->_alterSceneGlobal(state, out);
}

//helper
void MapNavigator::_alterScene(const MapHint::Alteration &alteration, const QList<QGraphicsItem*> &elements) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elements));
}

//helper
QList<Asset> MapNavigator::_fetchAssets(const QList<QGraphicsItem*> &listToFetch) const {
    QList<Asset> list;

    for(auto &e : listToFetch) {
        list.append(this->_hints->_assetsById[this->_idsByGraphicItem[e]]);
    }

    return list;
}

//from external instructions
void MapNavigator::alterScene(const QList<QUuid> &elementIds, const MapHint::Alteration &state) {
    
    this->_externalInstructionPending = true;

    //update internal state
    this->_hints->alterScene(elementIds, state);

    this->_externalInstructionPending = false;
}

//alter Scene
void MapNavigator::_alterSceneGlobal(const MapHint::Alteration &alteration, QList<Asset> &assets) { 
    
    //make sure to clear selection before selecting new
    if(alteration == MapHint::Alteration::Selected) this->scene()->clearSelection();
    if(alteration == MapHint::Alteration::Removed) this->_deletionProcessing = true;

    this->_hints->_alterSceneGlobal(alteration, assets);

    this->_deletionProcessing = false;

}

QUuid MapNavigator::_defineId(const MapHint::Alteration &alteration, Asset &asset) {
    auto elemId = asset.id();
    if(elemId.isNull()) {
        elemId = alteration == MapHint::Alteration::Added ? QUuid::createUuid() : this->_idsByGraphicItem[asset.graphicsItem()];
        asset.setId(elemId);
    }
    return elemId;
}


//register actions
QUuid MapNavigator::_alterSceneInternal(const MapHint::Alteration &alteration, Asset &asset) {

    //default handling first
    auto elemId = this->_defineId(alteration, asset); 
    auto ownerId = asset.ownerId();
    
    //pass hints
    this->_hints->_alterSceneInternal(alteration, asset);

    switch(alteration) {
        
        //on addition
        case MapHint::Alteration::Added:

            //bind elem
            if(!this->_hints->_assetsById.contains(elemId)) {
                this->_idsByGraphicItem.insert(asset.graphicsItem(), elemId);
            }
            break;
        
        //on focus
        case MapHint::Alteration::Focused:
            this->centerOn(asset.graphicsItem());
            break;

        //on selection
        case MapHint::Alteration::Selected:
            asset.graphicsItem()->setSelected(true);
            break;

        //on removal
        case MapHint::Alteration::Removed:

            //remove from map
            delete asset.graphicsItem();

            //update 
            this->_idsByGraphicItem.remove(asset.graphicsItem());
            break;

    }

    return elemId;
}