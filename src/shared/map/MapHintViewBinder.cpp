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

    //inform moving
    this->alterSceneFromItems(AlterationPayload::Alteration::Moved, this->_itemsWhoNotifiedMovement.toList());

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
    auto mapToEvt = this->_fetchAtoms(this->scene()->selectedItems());
    this->_emitAlteration(AlterationPayload::Alteration::Selected, mapToEvt);

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
    this->_alterSceneGlobal(AlterationPayload::Alteration::Reset, mapDb.toAtoms());
    
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


void MapHintViewBinder::_shouldMakeDirty(const AlterationPayload::Alteration &state, QVector<RPZAtom> &elements) {
    
    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!AlterationPayload::networkAlterations.contains(state)) return;

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

    //inform !
    auto newAtom = RPZAtom(AtomBase::Type::Drawing, newPath, metadata);
    this->alterSceneFromAtom(AlterationPayload::Alteration::Added, newAtom);
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

    //reset transparency as it is not a dummy anymore
    temporaryItem->setOpacity(1);

    //activate notifications since it is in scene now
    auto notifier = dynamic_cast<MapViewItemsNotifier*>(temporaryItem);
    if(notifier) notifier->activateNotifications();

    //inform !
    auto newAtom = RPZAtom((AtomBase::Type)assetElem->type(), temporaryItem, metadata);
    this->alterSceneFromAtom(AlterationPayload::Alteration::Added, newAtom);
}

/////////////////////////////////
// END Atom insertion helpers //
/////////////////////////////////

/////////////////////////
// Integration handler //
/////////////////////////

void MapHintViewBinder::unpackFromNetworkReceived(const QVariantHash &package) {
    auto payload = AlterationPayload::fromVariantHash(package);
    this->_alterSceneGlobal(payload.alteration(), *payload.atoms());
}


QGraphicsItem* MapHintViewBinder::_buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom) {
    
    QGraphicsItem* newItem = nullptr;

    //depending on atomType...
    switch(atomToBuildFrom.type()) {
        
        //drawing...
        case AtomBase::Type::Drawing: {

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
        case AtomBase::Type::Object: {

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
        auto newGi = this->scene()->addGenericImageBasedItem(pathToFile, atom.metadata()->layer(), 1, pos);
        
        //replace bound graphic item to the appropriate RPZAtoms which are already stored 
        auto id = atom.id();
        atom.setGraphicsItem(newGi);
        this->_atomsById[id] = atom;

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

QVector<RPZAtom> MapHintViewBinder::_fetchAtoms(const QList<QGraphicsItem*> &listToFetch) const {
    QVector<RPZAtom> list;
    for(auto e : listToFetch) {
        auto atom = this->_fetchAtom(e);
        list.append(atom);
    }
    return list;
}

RPZAtom MapHintViewBinder::_fetchAtom(QGraphicsItem* graphicElem) const {
    //failsafe check
    if(!this->_idsByGraphicItem.contains(graphicElem)) {
        qWarning() << "Atoms : cannot fetch atom id by its graphic item !";
        return RPZAtom();
    }
    auto id = this->_idsByGraphicItem[graphicElem];

    //failsafe check
    if(!this->_atomsById.contains(id)) {
        qWarning() << "Atoms : cannot fetch atom by its id !";
        return RPZAtom();
    }
    auto atom = this->_atomsById[id];

    return atom;
}

void MapHintViewBinder::alterSceneFromItems(const AlterationPayload::Alteration &alteration, const QList<QGraphicsItem*> &elements) {
    return this->_alterSceneGlobal(alteration, this->_fetchAtoms(elements));
}

void MapHintViewBinder::alterSceneFromItem(const AlterationPayload::Alteration &alteration, QGraphicsItem* element) {
    return this->alterSceneFromAtom(alteration, this->_fetchAtom(element));
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
void MapHintViewBinder::_alterSceneGlobal(const AlterationPayload::Alteration &alteration, QVector<RPZAtom> &atoms) { 
    
    this->_externalInstructionPending = true;

    //on reset
    if(alteration == AlterationPayload::Alteration::Reset) {
        this->_idsByGraphicItem.clear();
        for(auto item : this->_boundGv->items()) {
            delete item;
        }
    }
    if(alteration == AlterationPayload::Alteration::Selected) this->scene()->clearSelection();
    if(alteration == AlterationPayload::Alteration::Removed) this->_deletionProcessing = true;

    MapHint::_alterSceneGlobal(alteration, atoms);

    this->_preventNetworkAlterationEmission = false;
    this->_deletionProcessing = false;
    this->_externalInstructionPending = false;

    //define dirty
    this->_shouldMakeDirty(alteration, atoms);

}

QGraphicsItem* MapHintViewBinder::_findBoundGraphicsItem(const AlterationPayload::Alteration &alteration, RPZAtom &atom) {

    //if already bound (inner source), immerdiate return
    QGraphicsItem * item = atom.graphicsItem(); 
    if(item) {
        return item;
    }
    
    //if graphics items cache contains a reference, returns it and bind it to the atom
    auto atomId = atom.id();
    if (this->_atomsById.contains(atomId)) {

        item = this->_atomsById[atomId].graphicsItem();
        atom.setGraphicsItem(item);

        return item;

    }

    //in case of build type and nothing found yet, create graphics item
    if(AlterationPayload::buildGraphicsItemAlterations.contains(alteration)) {
        
        item = this->_buildGraphicsItemFromAtom(atom);
        atom.setGraphicsItem(item);

        return item;
    }

    //if nothing found, warning
    qWarning() << "Map : cannot find nor create RPZAtom's GraphicsItem";
    return item;
}

//register actions
void MapHintViewBinder::_alterSceneInternal(const AlterationPayload::Alteration &alteration, RPZAtom &atom) {

    //find or create the graphics item
    auto gItem = this->_findBoundGraphicsItem(alteration, atom);
    
    //default handling
    MapHint::_alterSceneInternal(alteration, atom); 

    //by alteration
    switch(alteration) {
        
        //on addition
        case AlterationPayload::Alteration::Reset:
        case AlterationPayload::Alteration::Added: {
            if(!this->_idsByGraphicItem.contains(gItem)) {
                this->_idsByGraphicItem.insert(gItem, atom.id());
            }
        }
        break;
        
        //on focus
        case AlterationPayload::Alteration::Focused: {
            this->_boundGv->centerOn(gItem);
        }
        break;

        //on moving
        case AlterationPayload::Alteration::Moved: {
            auto destPos = atom.metadata()->pos();
            gItem->setPos(destPos);  
        }
        break;

        //on layer change
        case AlterationPayload::Alteration::LayerChange: {
            auto newLayer = atom.metadata()->layer();
            gItem->setZValue(newLayer);
        }
        break;

        //on selection
        case AlterationPayload::Alteration::Selected: {
            gItem->setSelected(true);
        }
        break;

        //on removal
        case AlterationPayload::Alteration::Removed: {
            delete gItem;
            this->_idsByGraphicItem.remove(gItem);
        }
        break;

    }
}

////////////////////////////
// END MapHint Overriding //
////////////////////////////