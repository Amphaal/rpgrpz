#include "MapHintViewBinder.h"

MapHintViewBinder::MapHintViewBinder(QGraphicsView* boundGv) : _boundGv(boundGv) {

    //default layer from settings
    this->setDefaultLayer(AppContext::settings()->defaultLayer());

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

void MapHintViewBinder::setDefaultLayer(int layer) {
    this->_defaultLayer = layer;
}

void MapHintViewBinder::_onSceneItemChanged(QGraphicsItem* item, int alteration) {

    auto c_alteration = (AlterationPayload::Alteration)alteration;
    
    //on moving...
    if(c_alteration == AlterationPayload::Alteration::Moved) {
        
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


    //generate args for payload
    QHash<QUuid, QPointF> coords;
    for( auto gi : this->_itemsWhoNotifiedMovement) {
        coords.insert(this->_fetchAtom(gi)->id(), gi->scenePos());
    }

    //inform moving
    this->alterScene(MovedPayload(coords));

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

    //emit event, no RPZAtom alteration necessary
    auto selectedAtoms = this->_fetchAtoms(this->scene()->selectedItems());

    //extract ids for payload
    QList<QUuid> selectedAtomIds;
    for(auto atom : selectedAtoms) {
        selectedAtomIds.append(atom->id());
    }

    this->_emitAlteration(
        SelectedPayload(selectedAtomIds)
    );

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
    mapDb.saveIntoFile(this->atoms());

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
    this->_alterSceneGlobal(
        ResetPayload(mapDb.toAtoms())
    );
    
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


void MapHintViewBinder::_shouldMakeDirty(AlterationPayload &payload) {
    
    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!AlterationPayload::networkAlterations.contains(payload.type())) return;

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
        auto gi = this->_atomsById[elemId].graphicsItem();
        
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
// Atom insertion helpers //
/////////////////////////////

void MapHintViewBinder::addDrawing(const QPainterPath &path, const QPen &pen) {
            
    //translate to positionnable item
    auto initialPos = path.elementAt(0);
    auto translated = path.translated(-initialPos);

    //generate item
    auto newPath = this->scene()->addDrawing(translated, pen, initialPos);

    //define metadata
    auto metadata = RPZAtomMetadata();
    metadata.setPenWidth(pen.width());
    metadata.setLayer(this->_defaultLayer);
    metadata.setShape(newPath->path());
    metadata.setPos(initialPos);

    //inform !
    auto newAtom = RPZAtom(RPZAtom::Type::Drawing, newPath, metadata);
    this->alterScene(AddedPayload(newAtom));
}

QGraphicsItem* MapHintViewBinder::generateGhostItem(AssetsDatabaseElement* assetElem) {
    
    //find filepath to asset
    auto path = AssetsDatabase::get()->getFilePathToAsset(assetElem);
    auto temporaryItem = this->scene()->addGenericImageBasedItem(path, this->_defaultLayer, .5);

    //prevent notifications on move to kick in since the graphics item is not really in the scene
    auto notifier = dynamic_cast<MapViewItemsNotifier*>(temporaryItem);
    if(notifier) notifier->disableNotifications();

    return temporaryItem;
}

void MapHintViewBinder::turnGhostItemIntoDefinitive(QGraphicsItem* temporaryItem, AssetsDatabaseElement* assetElem, const QPoint &dropPos) {

    //prevent if remote
    if(this->_isRemote) return;

    //define position
    this->centerGraphicsItemToPoint(temporaryItem, dropPos);

    //define metadata
    auto metadata = RPZAtomMetadata();
    metadata.setAssetId(assetElem->id());
    metadata.setAssetName(assetElem->displayName());
    metadata.setLayer(this->_defaultLayer);
    metadata.setPos(dropPos);
    metadata.setShape(temporaryItem->boundingRect());

    //reset transparency as it is not a dummy anymore
    temporaryItem->setOpacity(1);

    //activate notifications since it is in scene now
    auto notifier = dynamic_cast<MapViewItemsNotifier*>(temporaryItem);
    if(notifier) notifier->activateNotifications();

    //inform !
    auto newAtom = RPZAtom((RPZAtom::Type)assetElem->type(), temporaryItem, metadata);
    this->alterScene(AddedPayload(newAtom));
}

/////////////////////////////////
// END Atom insertion helpers //
/////////////////////////////////

/////////////////////////
// Integration handler //
/////////////////////////


QGraphicsItem* MapHintViewBinder::_buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom) {

    QGraphicsItem* newItem = nullptr;

    //depending on atomType...
    switch(atomToBuildFrom.type()) {
        
        //drawing...
        case RPZAtom::Type::Drawing: {

            //extract path
            auto path = atomToBuildFrom.metadata()->shape();
            auto pos = atomToBuildFrom.metadata()->pos();
            
            //define a ped
            QPen pen;

                //if no owner set, assume it is self
                if(atomToBuildFrom.owner().id().isNull()) {
                    pen.setColor(this->_penColor);
                } else {
                    pen.setColor(atomToBuildFrom.owner().color());
                }

                //set width
                pen.setWidth(atomToBuildFrom.metadata()->penWidth());

            //draw the form
            newItem = this->scene()->addDrawing(path, pen, pos);
            
        }
        break;

        //objects
        case RPZAtom::Type::Object: {

            //depending on presence in asset db...
            auto assetId = atomToBuildFrom.metadata()->assetId();
            QString pathToAssetFile = AssetsDatabase::get()->getFilePathToAsset(assetId);
            
            //get position
            auto pos = atomToBuildFrom.metadata()->pos();

            //is in db
            if(!pathToAssetFile.isNull()) {
                
                //add to view
                newItem = this->scene()->addGenericImageBasedItem(pathToAssetFile, this->_defaultLayer, 1, pos);

            } 
            
            //not in db, render the shape
            else {
                
                //add placeholder
                auto boundingRect = atomToBuildFrom.metadata()->shape().boundingRect();
                auto placeholder = this->scene()->addMissingAssetPH(boundingRect, pos);
                newItem = placeholder;

                //if first time the ID is encountered
                if(!this->_missingAssetsIdsFromDb.contains(assetId)) {

                    //add graphic item to list of items to replace at times
                    this->_missingAssetsIdsFromDb.insert(assetId, placeholder);
                    emit requestMissingAsset(assetId);
                }

            }
        }
        break;
    }

    //save pointer ref
    this->_crossBindingAtomWithGI(&atomToBuildFrom, newItem);
    return newItem;
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

        //find corresponding atom
        auto atom = this->_fetchAtom(gi);

        //create the new graphics item
        auto newGi = this->scene()->addGenericImageBasedItem(pathToFile, atom->metadata()->layer(), 1, pos);
        this->_crossBindingAtomWithGI(atom, newGi);

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

void MapHintViewBinder::_crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi) {
    atom->setGraphicsItem(gi);
    gi->setData(0, (long long)atom);
}

MapViewGraphicsScene* MapHintViewBinder::scene() {
    return (MapViewGraphicsScene*)this->_boundGv->scene();
}

QVector<RPZAtom*> MapHintViewBinder::_fetchAtoms(const QList<QGraphicsItem*> &listToFetch) const {
    QVector<RPZAtom*> list;
    for(auto e : listToFetch) {
        auto atom = this->_fetchAtom(e);
        list.append(atom);
    }
    return list;
}

RPZAtom* MapHintViewBinder::_fetchAtom(QGraphicsItem* graphicElem) const {
    return (RPZAtom*)graphicElem->data(0).toLongLong();
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
void MapHintViewBinder::_alterSceneGlobal(AlterationPayload &payload) { 
    
    this->_externalInstructionPending = true;

    //on reset
    auto type = payload.type();
    if(type == AlterationPayload::Alteration::Reset) {
        for(auto item : this->_boundGv->items()) {
            delete item;
        }
    }
    if(type == AlterationPayload::Alteration::Selected) this->scene()->clearSelection();
    if(type == AlterationPayload::Alteration::Removed) this->_deletionProcessing = true;

    MapHint::_alterSceneGlobal(payload);

    this->_preventNetworkAlterationEmission = false;
    this->_deletionProcessing = false;
    this->_externalInstructionPending = false;

    //define dirty
    this->_shouldMakeDirty(payload);

}


//register actions
RPZAtom* MapHintViewBinder::_alterSceneInternal(const AlterationPayload::Alteration &type, QUuid &targetedAtomId, QVariant &atomAlteration) {
   
    //default handling
    auto updatedAtom = MapHint::_alterSceneInternal(type, targetedAtomId, atomAlteration); 

    //by alteration
    switch(type) {
        
        //on addition
        case AlterationPayload::Alteration::Reset:
        case AlterationPayload::Alteration::Added: {
            this->_buildGraphicsItemFromAtom(*updatedAtom);
        }
        break;
        
        //on focus
        case AlterationPayload::Alteration::Focused: {
            this->_boundGv->centerOn(updatedAtom->graphicsItem());
        }
        break;

        //on moving
        case AlterationPayload::Alteration::Moved: {
            auto destPos = updatedAtom->metadata()->pos();
            updatedAtom->graphicsItem()->setPos(destPos);  
        }
        break;

        //on layer change
        case AlterationPayload::Alteration::LayerChanged: {
            auto newLayer = updatedAtom->metadata()->layer();
            updatedAtom->graphicsItem()->setZValue(newLayer);
        }
        break;

        //on selection
        case AlterationPayload::Alteration::Selected: {
            updatedAtom->graphicsItem()->setSelected(true);
        }
        break;

        //on removal
        case AlterationPayload::Alteration::Removed: {
            auto toRemove = updatedAtom->graphicsItem();
            updatedAtom->setGraphicsItem(nullptr);
            delete toRemove;
        }
        break;

    }

    return updatedAtom;
}

////////////////////////////
// END MapHint Overriding //
////////////////////////////