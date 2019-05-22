#include "MapHintViewBinder.h"

MapHintViewBinder::MapHintViewBinder(QGraphicsView* boundGv) : _boundGv(boundGv) {

    //on selection
    QObject::connect(
        this->_boundGv->scene(), &QGraphicsScene::selectionChanged,
        this, &MapHintViewBinder::_onSceneSelectionChanged
    );

    //define dirty
    QObject::connect(
        this, &MapHint::assetsAlteredForNetwork,
        this, &MapHintViewBinder::_shouldMakeDirty
    );

};

void MapHintViewBinder::mayWantToSavePendingState() {
    if(!this->_isDirty || this->_isRemote) return;

    //popup
    auto result = QMessageBox::warning(
        this->_boundGv, 
        this->_stateFilePath, 
        "Voulez-vous sauvegarder les modifications effectuées sur la carte ?", 
        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes
    );

    //save state
    if(result == QMessageBox::Yes) {
        this->saveState();
    }

}

void MapHintViewBinder::_shouldMakeDirty(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements) {
    
    if(this->_isRemote) return;

    this->_setDirty();
}

bool MapHintViewBinder::loadDefaultState() {
    qDebug() << "Map : loading default map...";
    return this->loadState(getDefaultMapFile());
}

bool MapHintViewBinder::isRemote() {
    return this->_isRemote;
}

bool MapHintViewBinder::defineAsRemote(QString &remoteMapDescriptor) {
    this->_isRemote = !remoteMapDescriptor.isEmpty();
    if(this->_isRemote) this->_stateFilePath = remoteMapDescriptor;
    this->_setDirty(false);
    return this->_isRemote;
}

bool MapHintViewBinder::loadState(QString &filePath) {
    
    if(this->_isRemote) return false;

    //ask for save if dirty before loading
    this->mayWantToSavePendingState();

    //load file and parse it
    MapDatabase mapDb(filePath);
    this->_unpack(RPZAsset::Alteration::Reset, mapDb.toAssets());
    
    //change file path and define as clean
    this->_stateFilePath = filePath;
    this->_setDirty(false);

    return true;
}


bool MapHintViewBinder::saveState() {

    if(this->_isRemote) return false;

    //save into file
    MapDatabase mapDb(this->_stateFilePath);
    mapDb.saveIntoFile(this->fetchHistory());

    //define as clean
    this->_setDirty(false);

    return true;
}

QString MapHintViewBinder::stateFilePath() {
    return this->_stateFilePath;
}

bool MapHintViewBinder::isDirty() {
    return this->_isDirty;
}


void MapHintViewBinder::_setDirty(bool dirty) {
    this->_isDirty = dirty;
    emit mapFileStateChanged(this->_stateFilePath, this->_isDirty);
}

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

QGraphicsPathItem* MapHintViewBinder::_addDrawing(const QPainterPath &path, const QPen &pen) {
    
    //add path
    auto newPath = this->_boundGv->scene()->addPath(path, pen);
    
    //define flags
    newPath->setFlags(QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    ));

    return newPath;
}

void MapHintViewBinder::addDrawing(const QPainterPath &path, const QPen &pen) {
    
    auto newPath = this->_addDrawing(path, pen);

    //define metadata
    auto metadata = QVariantHash();
    metadata["w"] = pen.width();
    
    //inform !
    auto newAsset = RPZAsset(AssetBase::Type::Drawing, newPath, metadata);
    this->alterSceneFromAsset(RPZAsset::Alteration::Added, newAsset);
}



QGraphicsItem* MapHintViewBinder::generateTemplateAssetElement(AssetsDatabaseElement* assetElem) {
    //find filepath to asset
    auto path = AssetsDatabase::get()->getFilePathToAsset(assetElem);
    return this->_addGenericImageBasedAsset(path, .5);
}

QGraphicsItem* MapHintViewBinder::_addGenericImageBasedAsset(const QString &pathToImageFile, qreal opacity, const QPointF &initialPos) {
    
    //get file infos
    QFileInfo pathInfo(pathToImageFile);
    
    //define graphicsitem
    QGraphicsItem* item = nullptr;
    if(pathInfo.suffix() == "svg") {
        item = new QGraphicsSvgItem(pathToImageFile);
    } 
    else {
        auto pixmap = QPixmap(pathToImageFile);
        item = new QGraphicsPixmapItem(pixmap);
    };

    //define transparency as it is a dummy
    item->setOpacity(opacity);

    //define flags
    item->setFlags(QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    ));

    //add it to the scene
    this->_boundGv->scene()->addItem(item);

    //define position
    if(!initialPos.isNull()) item->setPos(initialPos);

    return item;
}

void MapHintViewBinder::addTemplateAssetElement(QGraphicsItem* temporaryItem, AssetsDatabaseElement* assetElem, const QPoint &dropPos) {

    //define position
    this->centerGraphicsItemToPoint(temporaryItem, dropPos);

    //define flags
    temporaryItem->setFlags(QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    ));

    //define metadata
    auto metadata = QVariantHash();
    metadata["a_id"] = assetElem->id();
    metadata["a_name"] = assetElem->displayName();

    //reset transparency as it is not a dummy anymore
    temporaryItem->setOpacity(1);

    //inform !
    auto newAsset = RPZAsset((AssetBase::Type)assetElem->type(), temporaryItem, metadata);
    this->alterSceneFromAsset(RPZAsset::Alteration::Added, newAsset);
}

void MapHintViewBinder::_unpack(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) {

    //if reset
    if(alteration == RPZAsset::Alteration::Reset) {
        this->_idsByGraphicItem.clear();
        for(auto item : this->_boundGv->items()) {
            delete item;
        }
    }

    //iterate through assets
    for(auto &asset : assets) {

        //if new element from network
        if(RPZAsset::mustParseGraphicsItem.contains(alteration)) {

            //newly created map elem
            QGraphicsItem* newItem = nullptr;

            //depending on assetType...
            switch(asset.type()) {
                
                //drawing...
                case AssetBase::Type::Drawing: {

                    //extract path
                    const QPainterPath path = JSONSerializer::fromBase64(*asset.data());
                    
                    //define a ped
                    QPen pen;
                    pen.setColor(asset.owner().color());
                    pen.setWidth(asset.metadata()->value("w").toInt());

                    //draw the form
                    newItem = this->_addDrawing(path, pen);
                }
                break;

                //objects
                case AssetBase::Type::Object: {

                    //depending on presence in asset db...
                    auto dbAssetId = asset.metadata()->value("a_id").toString();
                    QString pathToAssetFile = AssetsDatabase::get()->getFilePathToAsset(dbAssetId);
                                            
                    //extract the shape as bounding rect
                    auto boundingRect = JSONSerializer::fromBase64(*asset.data()).boundingRect();

                    //is in db
                    if(!pathToAssetFile.isNull()) {
                        
                        //add to view
                        newItem = this->_addGenericImageBasedAsset(pathToAssetFile, 1, boundingRect.topLeft());

                    } 
                    
                    //not in db, render the shape
                    else {
                        
                        //add placeholder
                        newItem = this->_addMissingAssetPH(boundingRect);

                        //TODO ask for missing assets
                    }
                }
                break;

                //
                default:
                    break;
            
            }

            asset.setGraphicsItem(newItem);

        }
    }

    //inform !
    this->_preventNetworkAlterationEmission = true;
    this->_alterSceneGlobal(alteration, assets);
}

QGraphicsRectItem* MapHintViewBinder::_addMissingAssetPH(QRectF &rect) {
    
    //pen to draw the rect with
    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setColor(Qt::GlobalColor::red);
    pen.setWidth(0);

    //background brush
    QBrush brush(QColor(255, 0, 0, 128));

    //add path
    auto placeholder = this->_boundGv->scene()->addRect(rect, pen, brush);
    
    //define flags
    placeholder->setFlags(QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    ));

    return placeholder;
}

void MapHintViewBinder::unpackFromNetworkReceived(const QVariantHash &package) {
    auto payload = AlterationPayload::fromVariantHash(package);
    this->_unpack(payload.alteration(), *payload.assets());
}

//helper
void MapHintViewBinder::alterSceneFromItems(const RPZAsset::Alteration &alteration, const QList<QGraphicsItem*> &elements) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elements));
}

//helper
QVector<RPZAsset> MapHintViewBinder::_fetchAssets(const QList<QGraphicsItem*> &listToFetch) const {
    QVector<RPZAsset> list;

    for(auto &e : listToFetch) {
        
        //failsafe check
        if(!this->_idsByGraphicItem.contains(e)) {
            qWarning() << "Assets : cannot fetch asset id by its graphic item !";
            continue;
        }
        auto id = this->_idsByGraphicItem[e];

        //failsafe check
        if(!this->_assetsById.contains(id)) {
            qWarning() << "Assets : cannot fetch asset by its id !";
            continue;
        }
        auto asset = this->_assetsById[id];

        //append
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
            qWarning() << "No graphicsItem found on cached Asset !";
        }
        
    } else {
        qWarning() << "Cannot find a graphicsItem bound to the asset !";
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
            if(item) this->_boundGv->centerOn(item);
            break;

        //on selection
        case RPZAsset::Alteration::Selected:
            if(item) item->setSelected(true);
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
