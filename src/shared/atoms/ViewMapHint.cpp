#include "ViewMapHint.h"

ViewMapHint::ViewMapHint(QGraphicsView* boundGv) : AtomsStorage(AlterationPayload::Source::Local_Map), _boundGv(boundGv) {

    //default layer from settings
    this->setDefaultLayer(AppContext::settings()->defaultLayer());

    //on selection
    QObject::connect(
        this->scene(), &QGraphicsScene::selectionChanged,
        this, &ViewMapHint::_onSceneSelectionChanged
    );

    //on scene children items changed
    QObject::connect(
        this->scene(), &MapViewGraphicsScene::sceneItemChanged,
        this, &ViewMapHint::_onSceneItemChanged
    ); 
};

bool ViewMapHint::isInTextInteractiveMode() {
    return this->_isInTextInteractiveMode;
}

void ViewMapHint::setDefaultLayer(int layer) {
    this->_defaultLayer = layer;
}

void ViewMapHint::handleAnyMovedItems() {
    
    //if no item moved since last call, do nothing
    if(!this->_itemsWhoNotifiedMovement.count()) return;

    //generate args for payload
    QHash<snowflake_uid, QPointF> coords;
    for( auto gi : this->_itemsWhoNotifiedMovement) {
        auto atom = this->_fetchAtom(gi);
        coords.insert(atom->id(), gi->pos());
    }

    //inform moving
    this->handleAlterationRequest(MovedPayload(coords));

    //enable notifications back on those items
    for(auto item : this->_itemsWhoNotifiedMovement) {
        auto notifier = dynamic_cast<MapViewItemsNotifier*>(item);
        if(notifier) notifier->activateNotifications();
    }

    //reset list 
    this->_itemsWhoNotifiedMovement.clear();
}

void ViewMapHint::_onSceneItemChanged(QGraphicsItem* item, int changeFlag) {

    if(this->_preventInnerGIEventsHandling) return;

    switch(changeFlag) {
        case MapViewCustomItemsEventFlag::Moved: {

            //add to list for future information
            this->_itemsWhoNotifiedMovement.insert(item);

            //disable further notifications until information have been handled
            auto notifier = dynamic_cast<MapViewItemsNotifier*>(item);
            if(notifier) notifier->disableNotifications();

        }
        break;

        case MapViewCustomItemsEventFlag::TextFocusIn: {
            this->_isInTextInteractiveMode = true;
        }
        break;
        
        case MapViewCustomItemsEventFlag::TextFocusOut: {

            this->_isInTextInteractiveMode = false;

            auto atom = this->_fetchAtom(item);
            auto cItem = (QGraphicsTextItem*)item;
            this->handleAlterationRequest(TextChangedPayload(atom->id(), cItem->toPlainText()));
        }
        break;
    }

}

void ViewMapHint::_onSceneSelectionChanged() {
    
    auto selectedAtoms = this->_fetchAtoms(this->scene()->selectedItems());

    //send selection changed
    QVector<void*> blandList;
    for(auto atom : selectedAtoms) {
        blandList.append(atom);
    }
    emit selectionChanged(blandList);

    //prevent
    if(this->_preventInnerGIEventsHandling) return;

    //extract ids for payload
    QVector<snowflake_uid> selectedAtomIds;
    for(auto atom : selectedAtoms) {
        selectedAtomIds.append(atom->id());
    }

    //bypass internal
    this->_emitAlteration(
        SelectedPayload(selectedAtomIds)
    );

}

////////////////////
// State handling //
////////////////////

QString ViewMapHint::stateFilePath() {
    return this->_stateFilePath;
}

bool ViewMapHint::isRemote() {
    return this->_isRemote;
}

bool ViewMapHint::isDirty() {
    return this->_isDirty;
}

void ViewMapHint::mayWantToSavePendingState() {
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

bool ViewMapHint::saveState() {

    if(this->_isRemote) return false;

    //save into file
    MapDatabase mapDb(this->_stateFilePath);
    mapDb.saveIntoFile(this->_atomsById);

    //define as clean
    this->_setDirty(false);

    return true;
}


bool ViewMapHint::saveStateAs(QString &newFilePath) {
    if(this->_isRemote) return false;

    this->_stateFilePath = newFilePath;
    return this->saveState();

}

bool ViewMapHint::loadDefaultState() {
    return this->loadState(
        AppContext::getDefaultMapFile()
    );
}


bool ViewMapHint::loadState(QString &filePath) {
    
    if(this->_isRemote) return false;

    //ask for save if dirty before loading
    this->mayWantToSavePendingState();

    //load file and parse it
    MapDatabase mapDb(filePath);
    this->_handlePayload(
        ResetPayload(mapDb.toAtoms())
    );
    
    //change file path and define as clean
    this->_stateFilePath = filePath;
    this->_setDirty(false);

    return true;
}


bool ViewMapHint::defineAsRemote(QString &remoteMapDescriptor) {
    
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


void ViewMapHint::_shouldMakeDirty(AlterationPayload &payload) {
    
    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!payload.isNetworkRoutable()) return;

    this->_setDirty();
}


void ViewMapHint::_setDirty(bool dirty) {
    this->_isDirty = dirty;
    emit mapFileStateChanged(this->_stateFilePath, this->_isDirty);
}


////////////////////////
// END State handling //
////////////////////////

//////////////////
// Pen handling //
//////////////////

QPen ViewMapHint::getPen() const {
    return QPen(this->_penColor, this->_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

void ViewMapHint::setPenColor(QColor &color) {
    this->_penColor = color;

    //update self graphic path items with new color
    for(auto &elemId : this->_atomIdsByOwnerId[0]) {
        
        auto gi = this->_atomsById[elemId].graphicsItem();
        
        //determine if is a path type
        if(auto pathItem = dynamic_cast<QGraphicsPathItem*>(gi)) {
            auto c_pen = pathItem->pen();
            c_pen.setColor(color);
            pathItem->setPen(c_pen);
        } 
    }
}

void ViewMapHint::setPenSize(int size) {
    this->_penWidth = size;
}

//////////////////////
// END Pen handling //
//////////////////////

/////////////////////////////
// Atom insertion helpers //
/////////////////////////////

void ViewMapHint::deleteCurrentSelectionItems() {
    QVector<snowflake_uid> atomIdsToRemove;
    for(auto item : this->scene()->selectedItems()) {
        auto atom = this->_fetchAtom(item);
        atomIdsToRemove.append(atom->id());
    }
    this->handleAlterationRequest(RemovedPayload(atomIdsToRemove));
}


void ViewMapHint::addDrawing(const QPointF &startPos, const QPainterPath &path, const QPen &pen) {

    //define metadata
    auto metadata = RPZAtomMetadata();
    metadata.setPenWidth(pen.width());
    metadata.setLayer(this->_defaultLayer);
    metadata.setShape(path);
    metadata.setPos(startPos);

    //inform !
    auto newAtom = RPZAtom(RPZAtom::Type::Drawing, metadata);
    this->handleAlterationRequest(AddedPayload(newAtom));

}

QGraphicsTextItem* ViewMapHint::generateGhostTextItem() {

    auto temporaryItem = this->scene()->addText(this->_contextualizedMetadata());

    //define transparency as it is a dummy
    temporaryItem->setOpacity(.5);

    //prevent notifications on move to kick in since the graphics item is not really in the scene
    auto notifier = dynamic_cast<MapViewItemsNotifier*>(temporaryItem);
    if(notifier) notifier->disableNotifications();

    return temporaryItem;
}

void ViewMapHint::turnGhostTextIntoDefinitive(QGraphicsTextItem* temporaryText, const QPoint &eventPos) {
    
    this->centerGraphicsItemToPoint(temporaryText, eventPos);

    //define metadata
    auto metadata = RPZAtomMetadata();
    metadata.setPenWidth(temporaryText->font().pointSize());
    metadata.setPos(temporaryText->scenePos());
    metadata.setLayer(temporaryText->zValue());
    metadata.setText(temporaryText->toPlainText());

    //inform !
    auto newAtom = RPZAtom(RPZAtom::Type::Text, metadata);
    this->handleAlterationRequest(AddedPayload(newAtom));

    //DO NOT REMOVE, it will be removed my the map
}

RPZAtomMetadata ViewMapHint::_contextualizedMetadata() {
    RPZAtomMetadata out;

    out.setLayer(this->_defaultLayer);
    out.setPenWidth(this->_penWidth);

    return out;
}

QGraphicsItem* ViewMapHint::generateGhostItem(AssetsDatabaseElement* assetElem) {
    
    //find filepath to asset
    auto path = AssetsDatabase::get()->getFilePathToAsset(assetElem);


    auto temporaryItem = this->scene()->addGenericImageBasedItem(path, this->_contextualizedMetadata());
    
    //define transparency as it is a dummy
    temporaryItem->setOpacity(.5);

    //prevent notifications on move to kick in since the graphics item is not really in the scene
    auto notifier = dynamic_cast<MapViewItemsNotifier*>(temporaryItem);
    if(notifier) notifier->disableNotifications();

    return temporaryItem;
}

void ViewMapHint::turnGhostItemIntoDefinitive(QGraphicsItem* temporaryItem, AssetsDatabaseElement* assetElem, const QPoint &eventPos) {

    //prevent if remote
    if(this->_isRemote) return;

    this->centerGraphicsItemToPoint(temporaryItem, eventPos);

    //define metadata
    auto metadata = RPZAtomMetadata();
    metadata.setAssetId(assetElem->id());
    metadata.setAssetName(assetElem->displayName());
    metadata.setLayer(temporaryItem->zValue());
    metadata.setPos(temporaryItem->scenePos());
    metadata.setShape(temporaryItem->boundingRect());

    //inform !
    auto newAtom = RPZAtom((RPZAtom::Type)assetElem->type(), metadata);
    this->handleAlterationRequest(AddedPayload(newAtom));

    //remove ghost
    delete temporaryItem;
}

/////////////////////////////////
// END Atom insertion helpers //
/////////////////////////////////

/////////////////////////
// Integration handler //
/////////////////////////


QGraphicsItem* ViewMapHint::_buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom) {

    QGraphicsItem* newItem = nullptr;

    //depending on atomType...
    switch(atomToBuildFrom.type()) {
        
        //text...
        case RPZAtom::Type::Text: { 
            newItem = this->scene()->addText(atomToBuildFrom.metadata());
        }
        break;

        //drawing...
        case RPZAtom::Type::Drawing: {
            newItem = this->scene()->addDrawing(atomToBuildFrom, this->_penColor);
        }
        break;

        //objects
        case RPZAtom::Type::Object: {

            //depending on presence in asset db...
            auto metadata = atomToBuildFrom.metadata();
            auto assetId = metadata.assetId();
            QString pathToAssetFile = AssetsDatabase::get()->getFilePathToAsset(assetId);
        
            //is in db, add to view
            if(!pathToAssetFile.isNull()) {
                newItem = this->scene()->addGenericImageBasedItem(pathToAssetFile, metadata);
            } 
            
            //not in db, render the shape
            else {
                
                //add placeholder
                auto placeholder = this->scene()->addMissingAssetPH(metadata);
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

void ViewMapHint::replaceMissingAssetPlaceholders(const QString &assetId) {
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
        auto newGi = this->scene()->addGenericImageBasedItem(pathToFile, atom->metadata());
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

void ViewMapHint::_crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi) {
    atom->setGraphicsItem(gi);
    auto ptrValToAtom = (long long)atom;
    gi->setData(0, ptrValToAtom);
}

MapViewGraphicsScene* ViewMapHint::scene() {
    return (MapViewGraphicsScene*)this->_boundGv->scene();
}

QVector<RPZAtom*> ViewMapHint::_fetchAtoms(const QList<QGraphicsItem*> &listToFetch) const {
    QVector<RPZAtom*> list;
    for(auto e : listToFetch) {
        auto atom = this->_fetchAtom(e);
        list.append(atom);
    }
    return list;
}

RPZAtom* ViewMapHint::_fetchAtom(QGraphicsItem* graphicElem) const {
    auto ptrValToAtom = graphicElem->data(0).toLongLong();
    return (RPZAtom*)ptrValToAtom;
}

void ViewMapHint::centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos) {
    QPointF point = this->_boundGv->mapToScene(eventPos);
    point = point - item->boundingRect().center();
    item->setPos(point);
}

//////////////////////////
// END Internal Helpers //
//////////////////////////

////////////////////////
// AtomsStorage Overriding //
////////////////////////

//alter Scene
void ViewMapHint::_handlePayload(AlterationPayload &payload) { 

    //make sure to not perpetuate circular payloads
    auto payloadSource = payload.source();
    if(payloadSource == this->_source) return;

    this->_preventInnerGIEventsHandling = true;

    //on reset
    auto type = payload.type();
    if(type == AlterationPayload::Alteration::Selected) this->scene()->clearSelection();
    if(type == AlterationPayload::Alteration::Reset) {
        for(auto item : this->_boundGv->items()) {
            delete item;
        }
    }
    
    AtomsStorage::_handlePayload(payload);

    //define dirty
    this->_shouldMakeDirty(payload);

    this->_preventInnerGIEventsHandling = false;
}

//register actions
RPZAtom* ViewMapHint::_handlePayloadInternal(const AlterationPayload::Alteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) {
   
    //default handling
    auto updatedAtom = AtomsStorage::_handlePayloadInternal(type, targetedAtomId, atomAlteration); 

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
            auto destPos = updatedAtom->metadata().pos();
            updatedAtom->graphicsItem()->setPos(destPos);  
        }
        break;

        //on scaling
        case AlterationPayload::Alteration::Scaled: {
            auto destScale = updatedAtom->metadata().scale();
            updatedAtom->graphicsItem()->setScale(destScale);
        }
        break;

        // on locking change
        case AlterationPayload::Alteration::LockChanged: {
            auto locked = updatedAtom->metadata().isLocked();
            auto flags = !locked ? MapViewItemsNotifier::defaultFlags() : 0;
            updatedAtom->graphicsItem()->setFlags(flags);
        }
        break;
        
        // on changing visibility
        case AlterationPayload::Alteration::VisibilityChanged: {
            auto hidden = updatedAtom->metadata().isHidden();
            auto opacity = hidden ? .05 : 1;
            updatedAtom->graphicsItem()->setOpacity(opacity);
        }
        break;

        //on rotation
        case AlterationPayload::Alteration::Rotated: {
            auto destRotation = updatedAtom->metadata().rotation();
            updatedAtom->graphicsItem()->setRotation(destRotation);
        }
        break;

        //on text change
        case AlterationPayload::Alteration::TextChanged: {
            auto newText = updatedAtom->metadata().text();
            auto cItem = (QGraphicsTextItem*)updatedAtom->graphicsItem();
            cItem->setPlainText(newText);
        }
        break;

        //on layer change
        case AlterationPayload::Alteration::LayerChanged: {
            auto newLayer = updatedAtom->metadata().layer();
            updatedAtom->graphicsItem()->setZValue(newLayer);
        }
        break;

        //on selection
        case AlterationPayload::Alteration::Selected: {
            
            updatedAtom->graphicsItem()->setSelected(true);
        }
        break;

    }

    return updatedAtom;
}

////////////////////////////
// END AtomsStorage Overriding //
////////////////////////////