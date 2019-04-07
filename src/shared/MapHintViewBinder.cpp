#include "MapHintViewBinder.h"

MapHintViewBinder::MapHintViewBinder(QGraphicsView* boundGv) : _boundGv(boundGv) {

    //on selection
    QObject::connect(
        this->_boundGv->scene(), &QGraphicsScene::selectionChanged,
        this, &MapHintViewBinder::_onSceneSelectionChanged
    );

};

void MapHintViewBinder::_onSceneSelectionChanged() {

    if(this->_externalInstructionPending || this->_deletionProcessing) return;

    //emit event
    auto mapToEvt = this->_fetchAssets(this->_boundGv->scene()->selectedItems());
    this->_emitAlteration(MapHint::Alteration::Selected, mapToEvt);

}

void MapHintViewBinder::unpackFromNetworkReceived(const QVariantList &package) {

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
                    newItem = this->_boundGv->scene()->addPath(path);
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
void MapHintViewBinder::alterSceneFromItems(const MapHint::Alteration &alteration, const QList<QGraphicsItem*> &elements) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elements));
}

//helper
QList<Asset> MapHintViewBinder::_fetchAssets(const QList<QGraphicsItem*> &listToFetch) const {
    QList<Asset> list;

    for(auto &e : listToFetch) {
        auto id = this->_idsByGraphicItem[e];
        auto asset = this->_assetsById[id];
        list.append(asset);
    }

    return list;
}

//alter Scene
void MapHintViewBinder::_alterSceneGlobal(const MapHint::Alteration &alteration, QList<Asset> &assets) { 
    
    this->_externalInstructionPending = true;

    //make sure to clear selection before selecting new
    if(alteration == MapHint::Alteration::Selected) this->_boundGv->scene()->clearSelection();
    if(alteration == MapHint::Alteration::Removed) this->_deletionProcessing = true;

    MapHint::_alterSceneGlobal(alteration, assets);

    this->_deletionProcessing = false;
    this->_externalInstructionPending = false;

}

//overwrites
QUuid MapHintViewBinder::_defineId(const MapHint::Alteration &alteration, Asset &asset) {
    auto elemId = asset.id();
    if(elemId.isNull()) {
        elemId = alteration == MapHint::Alteration::Added ? QUuid::createUuid() : this->_idsByGraphicItem[asset.graphicsItem()];
        asset.setId(elemId);
    }
    return elemId;
}


//register actions
QUuid MapHintViewBinder::_alterSceneInternal(const MapHint::Alteration &alteration, Asset &asset) {

    //default handling first
    auto elemId = MapHint::_alterSceneInternal(alteration, asset); 
    auto item = asset.graphicsItem();

    switch(alteration) {
        
        //on addition
        case MapHint::Alteration::Added:

            //bind elem
            if(!this->_idsByGraphicItem.contains(item)) {
                this->_idsByGraphicItem.insert(item, elemId);
            }
            break;
        
        //on focus
        case MapHint::Alteration::Focused:
            this->_boundGv->centerOn(item);
            break;

        //on selection
        case MapHint::Alteration::Selected:
            item->setSelected(true);
            break;

        //on removal
        case MapHint::Alteration::Removed:

            //remove from map
            delete item;

            //update 
            this->_idsByGraphicItem.remove(item);
            break;

    }

    return elemId;
}