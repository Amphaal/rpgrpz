#include "MapHintViewBinder.h"

MapHintViewBinder::MapHintViewBinder(QGraphicsView* boundGv) : _boundGv(boundGv) {

    //on selection
    QObject::connect(
        this->scene(), &QGraphicsScene::selectionChanged,
        this, &MapHintViewBinder::_onSceneSelectionChanged
    );

    //on scene children items changed
    QObject::connect(
        this->scene(), &MapViewGraphicsScene::sceneItemChanged,
        this, &MapHintViewBinder::_onSceneItemChanged
    ); 

};

void MapHintViewBinder::_onSceneItemChanged(QGraphicsItem* item, int alteration) {

    auto c_alteration = (RPZAsset::Alteration)alteration;
    
    //on moving...
    if(c_alteration == RPZAsset::Alteration::Moved) {
        
        //add to list for future information
        this->_itemsWhoNotifiedMovement.insert(item);

        //disable further notifications until information have been handled
        auto notifier = dynamic_cast<MapViewItemsNotifier*>(item);
        if(notifier) notifier->disableNotifications();
    }
}

void MapHintViewBinder::handleAnyMovedItems() {
    
    //if no item moved since last call, do nothing
    if(!this->_itemsWhoNotifiedMovement.count()) return;

    //if instructions pending, abort
    if(this->_externalInstructionPending || this->_deletionProcessing) return;

    //inform moving
    this->alterSceneFromItems(RPZAsset::Alteration::Moved, this->_itemsWhoNotifiedMovement.toList());

    //enable notifications back on those items
    for(auto item : this->_itemsWhoNotifiedMovement) {
        auto notifier = dynamic_cast<MapViewItemsNotifier*>(item);
        if(notifier) notifier->activateNotifications();
    }

    //reset list 
    this->_itemsWhoNotifiedMovement.clear();
}

void MapHintViewBinder::_onSceneSelectionChanged() {

    if(this->_externalInstructionPending || this->_deletionProcessing) return;

    //emit event, no RPZAsset alteration necessary
    auto mapToEvt = this->_fetchAssets(this->scene()->selectedItems());
    this->_emitAlteration(RPZAsset::Alteration::Selected, mapToEvt);

}

////////////////////
// State handling //
////////////////////

QString MapHintViewBinder::stateFilePath() {
    return this->_stateFilePath;
}

bool MapHintViewBinder::isRemote() {
    return this->_isRemote;
}

bool MapHintViewBinder::isDirty() {
    return this->_isDirty;
}

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

bool MapHintViewBinder::saveState() {

    if(this->_isRemote) return false;

    //save into file
    MapDatabase mapDb(this->_stateFilePath);
    mapDb.saveIntoFile(this->fetchHistory());

    //define as clean
    this->_setDirty(false);

    return true;
}


bool MapHintViewBinder::saveStateAs(QString &newFilePath) {
    if(this->_isRemote) return false;

    this->_stateFilePath = newFilePath;
    return this->saveState();

}

bool MapHintViewBinder::loadDefaultState() {
    qDebug() << "Map : loading default map...";
    return this->loadState(
        AppContext::getDefaultMapFile()
    );
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


bool MapHintViewBinder::defineAsRemote(QString &remoteMapDescriptor) {
    
    //define remote flag
    this->_isRemote = !remoteMapDescriptor.isEmpty();
    
    //reset missing assets list
    this->_missingAssetsIdsFromDb.clear();

    //change map descriptor if is a remote session
    if(this->_isRemote) this->_stateFilePath = remoteMapDescriptor;

    //anyway, unset dirty
    this->_setDirty(false);
    
    return this->_isRemote;
}


void MapHintViewBinder::_shouldMakeDirty(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements) {
    
    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!RPZAsset::networkAlterations.contains(state)) return;

    this->_setDirty();
}


void MapHintViewBinder::_setDirty(bool dirty) {
    this->_isDirty = dirty;
    emit mapFileStateChanged(this->_stateFilePath, this->_isDirty);
}


////////////////////////
// END State handling //
////////////////////////

//////////////////
// Pen handling //
//////////////////

QPen MapHintViewBinder::getPen() const {
    return QPen(this->_penColor, this->_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

void MapHintViewBinder::setPenColor(QColor &color) {
    this->_penColor = color;

    //update self graphic path items with new color
    for(auto &elemId : this->_selfElements) {
        auto gi = this->_assetsById[elemId].graphicsItem();
        
        //determine if is a path type
        auto casted = dynamic_cast<QGraphicsPathItem*>(gi);
        if(!casted) continue;

        //update color
        auto c_pen = casted->pen();
        c_pen.setColor(color);
        casted->setPen(c_pen);
    }
}

void MapHintViewBinder::setPenSize(int size) {
    this->_penWidth = size;
}

//////////////////////
// END Pen handling //
//////////////////////

/////////////////////////////
// Asset insertion helpers //
/////////////////////////////

void MapHintViewBinder::addDrawing(const QPainterPath &path, const QPen &pen) {
            
    //translate to positionnable item
    auto initialPos = path.elementAt(0);
    auto translated = path.translated(-initialPos);

    //generate item
    auto newPath = this->scene()->addDrawing(translated, pen, initialPos);

    //define metadata
    auto metadata = RPZAssetMetadata();
    metadata.setPenWidth(pen.width());

    //inform !
    auto newAsset = RPZAsset(AssetBase::Type::Drawing, newPath, metadata);
    this->alterSceneFromAsset(RPZAsset::Alteration::Added, newAsset);
}

QGraphicsItem* MapHintViewBinder::generateTemplateAssetElement(AssetsDatabaseElement* assetElem) {
    
    //find filepath to asset
    auto path = AssetsDatabase::get()->getFilePathToAsset(assetElem);
    auto temporaryItem = this->scene()->addGenericImageBasedAsset(path, .5);

    //prevent notifications on move to kick in since the asset is not really in the scene
    auto notifier = dynamic_cast<MapViewItemsNotifier*>(temporaryItem);
    if(notifier) notifier->disableNotifications();

    return temporaryItem;
}

void MapHintViewBinder::addTemplateAssetElement(QGraphicsItem* temporaryItem, AssetsDatabaseElement* assetElem, const QPoint &dropPos) {

    //prevent if remote
    if(this->_isRemote) return;

    //define position
    this->centerGraphicsItemToPoint(temporaryItem, dropPos);

    //define metadata
    auto metadata = RPZAssetMetadata();
    metadata.setDbAssetId(assetElem->id());
    metadata.setDbAssetName(assetElem->displayName());

    //reset transparency as it is not a dummy anymore
    temporaryItem->setOpacity(1);

    //activate notifications since it is in scene now
    auto notifier = dynamic_cast<MapViewItemsNotifier*>(temporaryItem);
    if(notifier) notifier->activateNotifications();

    //inform !
    auto newAsset = RPZAsset((AssetBase::Type)assetElem->type(), temporaryItem, metadata);
    this->alterSceneFromAsset(RPZAsset::Alteration::Added, newAsset);
}

/////////////////////////////////
// END Asset insertion helpers //
/////////////////////////////////

/////////////////////////
// Integration handler //
/////////////////////////

void MapHintViewBinder::unpackFromNetworkReceived(const QVariantHash &package) {
    auto payload = AlterationPayload::fromVariantHash(package);
    this->_unpack(payload.alteration(), *payload.assets());
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
        
        //newly created map elem
        QGraphicsItem* newItem = nullptr;

        //conditionnal handling
        if(RPZAsset::buildGraphicsItemAlterations.contains(alteration)) {

            //if a fresh graphics item must be built
            newItem = this->_unpack_build(asset);

        } else if (RPZAsset::updateGraphicsItemAlterations.contains(alteration)) {
            
            //if it is just a mere update on the bound graphics item
            newItem = this->_unpack_update(alteration, asset);

        }

        //replace
        if(newItem) {
            asset.setGraphicsItem(newItem);
        }
    }

    //inform !
    this->_preventNetworkAlterationEmission = true;
    this->_alterSceneGlobal(alteration, assets);
}

QGraphicsItem* MapHintViewBinder::_unpack_build(RPZAsset &assetToBuildFrom) {
    
    QGraphicsItem* newItem = nullptr;

    //depending on assetType...
    switch(assetToBuildFrom.type()) {
        
        //drawing...
        case AssetBase::Type::Drawing: {

            //extract path
            auto path = assetToBuildFrom.metadata()->shape();
            auto pos = assetToBuildFrom.metadata()->pos();
            
            //define a ped
            QPen pen;

                //if no owner set, assume it is self
                if(assetToBuildFrom.owner().id().isNull()) {
                    pen.setColor(this->_penColor);
                } else {
                    pen.setColor(assetToBuildFrom.owner().color());
                }

                //set width
                pen.setWidth(assetToBuildFrom.metadata()->penWidth());

            //draw the form
            newItem = this->scene()->addDrawing(path, pen, pos);
        }
        break;

        //objects
        case AssetBase::Type::Object: {

            //depending on presence in asset db...
            auto dbAssetId = assetToBuildFrom.metadata()->dbAssetId();
            QString pathToAssetFile = AssetsDatabase::get()->getFilePathToAsset(dbAssetId);
            
            //get position
            auto pos = assetToBuildFrom.metadata()->pos();

            //is in db
            if(!pathToAssetFile.isNull()) {
                
                //add to view
                newItem = this->scene()->addGenericImageBasedAsset(pathToAssetFile, 1, pos);

            } 
            
            //not in db, render the shape
            else {
                
                //add placeholder
                auto boundingRect = assetToBuildFrom.metadata()->shape().boundingRect();
                auto placeholder = this->scene()->addMissingAssetPH(boundingRect, pos);
                newItem = placeholder;

                //if first time the ID is encountered
                if(!this->_missingAssetsIdsFromDb.contains(dbAssetId)) {

                    //add graphic item to list of items to replace at times
                    this->_missingAssetsIdsFromDb.insert(dbAssetId, placeholder);
                    emit requestMissingAsset(dbAssetId);
                }

            }
        }
        break;
    }

    return newItem;
}

QGraphicsItem* MapHintViewBinder::_unpack_update(const RPZAsset::Alteration &alteration, RPZAsset &assetToUpdateFrom) {
    
    //get the gi
    QGraphicsItem* itemToUpdate = this->_idsByGraphicItem.key(assetToUpdateFrom.id());

    switch(alteration) { 
        case RPZAsset::Alteration::Moved: {    
            auto destPos = assetToUpdateFrom.metadata()->pos();
            itemToUpdate->setPos(destPos);
        }
        break;
        case RPZAsset::Alteration::LayerChange: {
            auto newLayer = assetToUpdateFrom.metadata()->layer();
            itemToUpdate->setZValue(newLayer);
        }
        break;
    }

    return itemToUpdate;
}

void MapHintViewBinder::replaceMissingAssetPlaceholders(const QString &assetId) {
    if(!this->_missingAssetsIdsFromDb.contains(assetId)) return;

    //find the path file
    auto pathToFile = AssetsDatabase::get()->getFilePathToAsset(QString(assetId));
    if(pathToFile.isNull()) return;
    
    //iterate through the list of GI to replace
    auto setOfGraphicsItemsToReplace = this->_missingAssetsIdsFromDb.values(assetId).toSet();
    for(auto gi : setOfGraphicsItemsToReplace) {
        
        //add on top the required graphical item
        auto pos = gi->scenePos();
        if(pos.isNull()) {
            pos = gi->sceneBoundingRect().topLeft();
        }
        auto newGi = this->scene()->addGenericImageBasedAsset(pathToFile, 1, pos);
        
        //replace bound graphic item to the appropriate RPZAssets which are already stored 
        auto id = this->_idsByGraphicItem[gi];
        auto asset = this->_assetsById[id];
        asset.setGraphicsItem(newGi);
        this->_assetsById[id] = asset;

        //delete placeholder
        this->_idsByGraphicItem.remove(gi);
        this->_idsByGraphicItem.insert(newGi, id);
        delete gi;
    }

    //clear the id from the missing list
    this->_missingAssetsIdsFromDb.remove(assetId);
}

/////////////////////////////
// END Integration handler //
/////////////////////////////

//////////////////////
// Internal Helpers //
//////////////////////

MapViewGraphicsScene* MapHintViewBinder::scene() {
    return (MapViewGraphicsScene*)this->_boundGv->scene();
}

QVector<RPZAsset> MapHintViewBinder::_fetchAssets(const QList<QGraphicsItem*> &listToFetch) const {
    QVector<RPZAsset> list;
    for(auto e : listToFetch) {
        auto asset = this->_fetchAsset(e);
        list.append(asset);
    }
    return list;
}

RPZAsset MapHintViewBinder::_fetchAsset(QGraphicsItem* graphicElem) const {
    //failsafe check
    if(!this->_idsByGraphicItem.contains(graphicElem)) {
        qWarning() << "Assets : cannot fetch asset id by its graphic item !";
        return RPZAsset();
    }
    auto id = this->_idsByGraphicItem[graphicElem];

    //failsafe check
    if(!this->_assetsById.contains(id)) {
        qWarning() << "Assets : cannot fetch asset by its id !";
        return RPZAsset();
    }
    auto asset = this->_assetsById[id];

    return asset;
}

void MapHintViewBinder::alterSceneFromItems(const RPZAsset::Alteration &alteration, const QList<QGraphicsItem*> &elements) {
    return this->_alterSceneGlobal(alteration, this->_fetchAssets(elements));
}

void MapHintViewBinder::alterSceneFromItem(const RPZAsset::Alteration &alteration, QGraphicsItem* element) {
    return this->alterSceneFromAsset(alteration, this->_fetchAsset(element));
}

void MapHintViewBinder::centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos) {
    QPointF point = this->_boundGv->mapToScene(eventPos);
    point = point - item->boundingRect().center();
    item->setPos(point);
}

//////////////////////////
// END Internal Helpers //
//////////////////////////

////////////////////////
// MapHint Overriding //
////////////////////////

//alter Scene
void MapHintViewBinder::_alterSceneGlobal(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) { 
    
    this->_externalInstructionPending = true;

    //make sure to clear selection before selecting new
    if(alteration == RPZAsset::Alteration::Selected) this->scene()->clearSelection();
    if(alteration == RPZAsset::Alteration::Removed) this->_deletionProcessing = true;

    MapHint::_alterSceneGlobal(alteration, assets);

    this->_preventNetworkAlterationEmission = false;
    this->_deletionProcessing = false;
    this->_externalInstructionPending = false;

    //define dirty
    this->_shouldMakeDirty(alteration, assets);

}

QGraphicsItem* MapHintViewBinder::_findBoundGraphicsItem(RPZAsset &asset) {
    
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

    return item;
}

//register actions
QUuid MapHintViewBinder::_alterSceneInternal(const RPZAsset::Alteration &alteration, RPZAsset &asset) {

    //get underlying item. If received from network, search in local cache
    auto elemId = asset.id();
    QGraphicsItem * item = this->_findBoundGraphicsItem(asset);
    
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

////////////////////////////
// END MapHint Overriding //
////////////////////////////