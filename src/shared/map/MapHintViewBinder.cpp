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
    this->_emitAlteration(RPZAsset::Alteration::Selected, mapToEvt);

}

void MapHintViewBinder::centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos) {
    QPointF point = this->_boundGv->mapToScene(eventPos);
    point = point - item->boundingRect().center();
    item->setPos(point);
}

QGraphicsPathItem* MapHintViewBinder::addDrawing(const QPainterPath &path, const QPen &pen) {
    
    //add path
    auto newPath = this->_boundGv->scene()->addPath(path, pen);
    
    //define flags
    newPath->setFlags(QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    ));

    //define metadata
    auto metadata = QVariantHash();
    metadata["w"] = pen.width();

    //inform
    auto newAsset = RPZAsset(AssetBase::Type::Drawing, newPath, metadata);
    this->alterSceneFromAsset(RPZAsset::Alteration::Added, newAsset);

    return newPath;
}

QGraphicsItem* MapHintViewBinder::temporaryAssetElement(AssetsDatabaseElement* assetElem, AssetsDatabase *database) {
        
        //find filepath to asset
        auto path = database->getFilePathToAsset(assetElem);
        QFileInfo pathInfo(path);
        
        //define graphicsitem
        QGraphicsItem* item = nullptr;
        if(pathInfo.suffix() == "svg") {
            item = new QGraphicsSvgItem(path);
        } 
        else {
            item = new QGraphicsPixmapItem(QPixmap(path));
        };

        //define transparency as it is a dummy
        item->setOpacity(.5);

        //add it to the scene
        this->_boundGv->scene()->addItem(item);

        return item;
}



QGraphicsItem* MapHintViewBinder::addAssetElement(QGraphicsItem* item, AssetsDatabaseElement* assetElem, const QPoint &pos) {

        //define position
        this->centerGraphicsItemToPoint(item, pos);

        //define flags
        item->setFlags(QFlags<QGraphicsItem::GraphicsItemFlag>(
            QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
            QGraphicsItem::GraphicsItemFlag::ItemIsMovable
        ));

        //inform
        auto newAsset = RPZAsset((AssetBase::Type)assetElem->type(), item);
        this->alterSceneFromAsset(RPZAsset::Alteration::Added, newAsset);
    
        //reset transparency as it is not a dummy anymore
        item->setOpacity(1);

        return item;
}

void MapHintViewBinder::unpackFromNetworkReceived(const QVariantHash &package) {

    auto payload = AlterationPayload::fromVariantHash(package);

    //if reset
    if(payload.alteration() == RPZAsset::Alteration::Reset) {
        this->_idsByGraphicItem.clear();
        for(auto item : this->_boundGv->items()) {
            delete item;
        }
    }

    //iterate through assets
    for(auto &asset : *payload.assets()) {

        //if new element from network
        if(RPZAsset::mustParseGraphicsItem.contains(payload.alteration())) {

            //newly created map elem
            QGraphicsItem * newItem;

            //depending on assetType...
            switch(asset.type()) {
                
                //drawing...
                case AssetBase::Type::Drawing:
                    const QPainterPath path = JSONSerializer::fromBase64(*asset.data());
                    auto pen = QPen();
                    pen.setColor(asset.owner().color());
                    pen.setWidth(asset.metadata()->value("w").toInt());
                    newItem = this->addDrawing(path, pen);
                    break;
            
            }

            asset.setGraphicsItem(newItem);

        }
    }

    //process new state
    this->_preventNetworkAlterationEmission = true;
    this->_alterSceneGlobal(payload.alteration(), *payload.assets());
}

//helper
void MapHintViewBinder::alterSceneFromItems(const RPZAsset::Alteration &alteration, const QList<QGraphicsItem*> &elements) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elements));
}

//helper
QVector<RPZAsset> MapHintViewBinder::_fetchAssets(const QList<QGraphicsItem*> &listToFetch) const {
    QVector<RPZAsset> list;

    for(auto &e : listToFetch) {
        auto id = this->_idsByGraphicItem[e];
        auto asset = this->_assetsById[id];
        list.append(asset);
    }

    return list;
}

//alter Scene
void MapHintViewBinder::_alterSceneGlobal(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) { 
    
    this->_externalInstructionPending = true;

    //make sure to clear selection before selecting new
    if(alteration == RPZAsset::Alteration::Selected) this->_boundGv->scene()->clearSelection();
    if(alteration == RPZAsset::Alteration::Removed) this->_deletionProcessing = true;


    MapHint::_alterSceneGlobal(alteration, assets);

    this->_preventNetworkAlterationEmission = false;
    this->_deletionProcessing = false;
    this->_externalInstructionPending = false;

}

//register actions
QUuid MapHintViewBinder::_alterSceneInternal(const RPZAsset::Alteration &alteration, RPZAsset &asset) {

    //get underlying item. If received from network, search in local cache
    auto elemId = asset.id();
    QGraphicsItem * item = nullptr; 
    if(asset.graphicsItem()) {

        item = asset.graphicsItem();

    } else if (this->_assetsById.contains(elemId)) {

        auto cachedAsset = this->_assetsById[elemId];

        if(cachedAsset.graphicsItem()) {
            
            item = this->_assetsById[elemId].graphicsItem();
        
        } else {
            qDebug() << "No graphicsItem found on cached Asset !";
        }
        
    } else {
        qDebug() << "Cannot find a graphicsItem bound to the asset !";
    }
    
    //default handling last, cuz asset will be deleted first from internal list
    MapHint::_alterSceneInternal(alteration, asset); 

    switch(alteration) {
        
        //on addition
        case RPZAsset::Alteration::Reset:
        case RPZAsset::Alteration::Added:

            //bind elem
            if(!this->_idsByGraphicItem.contains(item)) {
                this->_idsByGraphicItem.insert(item, elemId);
            }
            break;
        
        //on focus
        case RPZAsset::Alteration::Focused:
            this->_boundGv->centerOn(item);
            break;

        //on selection
        case RPZAsset::Alteration::Selected:
            item->setSelected(true);
            break;

        //on removal
        case RPZAsset::Alteration::Removed:

            //remove from map
            if(item) delete item;

            //update 
            this->_idsByGraphicItem.remove(item);
            break;

    }

    return elemId;
}