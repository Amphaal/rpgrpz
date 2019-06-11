#include "ViewMapHint.h"

ViewMapHint::ViewMapHint(QGraphicsView* boundGv) : AtomsStorage(AlterationPayload::Source::Local_Map), 
    AtomsContextualMenuHandler(this, boundGv), 
    _boundGv(boundGv),
    _templateAtom(new RPZAtom) {

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
    this->_templateAtom->setLayer(layer);
    emit atomTemplateChanged(this->_templateAtom);
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
        case (int)MapViewCustomItemsEventFlag::Moved: {

            //add to list for future information
            this->_itemsWhoNotifiedMovement.insert(item);

            //disable further notifications until information have been handled
            auto notifier = dynamic_cast<MapViewItemsNotifier*>(item);
            if(notifier) notifier->disableNotifications();

        }
        break;

        case (int)MapViewCustomItemsEventFlag::TextFocusIn: {
            this->_isInTextInteractiveMode = true;
        }
        break;
        
        case (int)MapViewCustomItemsEventFlag::TextFocusOut: {

            this->_isInTextInteractiveMode = false;

            auto atom = this->_fetchAtom(item);
            auto cItem = (QGraphicsTextItem*)item;
            this->handleAlterationRequest(TextChangedPayload(atom->id(), cItem->toPlainText()));
        }
        break;
    }

}

QVector<snowflake_uid> ViewMapHint::_selectedAtomIds() {
    
    auto selectedAtoms = this->_fetchAtoms(this->scene()->selectedItems());
    
    QVector<snowflake_uid> selectedAtomIds;
    for(auto atom : selectedAtoms) {
        selectedAtomIds.append(atom->id());
    }
    
    return selectedAtomIds;
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
    auto payload = SelectedPayload(selectedAtomIds);
    this->_emitAlteration(&payload);

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
    this->handleAlterationRequest(
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


void ViewMapHint::_shouldMakeDirty(AlterationPayload* payload) {
    
    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!payload->isNetworkRoutable()) return;

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
    return QPen(
        this->_templateAtom->owner().color(), 
        this->_templateAtom->penWidth(), 
        Qt::SolidLine, 
        Qt::RoundCap, 
        Qt::RoundJoin
    );
}

void ViewMapHint::setDefaultUser(RPZUser user) {
    
    //update template
    auto oldOwnerId = this->_templateAtom->owner().id();
    this->_templateAtom->setOwnership(user);
    emit atomTemplateChanged(this->_templateAtom);

    //update self graphic path items with new color
    auto color = user.color();

    auto atomIds = this->_atomIdsByOwnerId[oldOwnerId];
    for(auto &elemId : atomIds) {
        
        auto &atom = this->_atomsById[elemId];
        auto gi = atom.graphicsItem();
        
        //determine if is a path type
        if(auto pathItem = dynamic_cast<QGraphicsPathItem*>(gi)) {
            auto c_pen = pathItem->pen();
            c_pen.setColor(color);
            pathItem->setPen(c_pen);
        } 
        
        //redefine ownership
        atom.setOwnership(user);
    }

    //change bound
    this->_atomIdsByOwnerId.remove(oldOwnerId);
    this->_atomIdsByOwnerId.insert(user.id(), atomIds);

    //inform atom layout
    auto payload = OwnerChangedPayload(atomIds.toList().toVector(), user);
    this->_emitAlteration(&payload);

}


void ViewMapHint::setPenSize(int size) {
    this->_templateAtom->setPenWidth(size);
    emit atomTemplateChanged(this->_templateAtom);
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

QGraphicsItem* ViewMapHint::generateGhostItem(const AtomType &type, const QString assetId, const QString assetName, const QString assetLocation) {

    //update template
    this->_templateAtom->changeType(type);
    this->_templateAtom->setAssetId(assetId);
    this->_templateAtom->setAssetName(assetName);
    
    //generate a blueprint
    auto atomBuiltFromTemplate = RPZAtom(*this->_templateAtom);
    atomBuiltFromTemplate.setLayer(
        atomBuiltFromTemplate.layer() + 1
    ); //add +1 to layer, will be discarded later

    //add to scene
    auto aditionnalArgs = MVPayload(assetLocation);
    QGraphicsItem* ghostItem = this->scene()->addToScene(atomBuiltFromTemplate, aditionnalArgs);

    //if no ghost item, return
    if(!ghostItem) return ghostItem;

    //define transparency as it is a dummy
    ghostItem->setOpacity(.5);

    //prevent notifications on move to kick in since the graphics item is not really in the scene
    auto notifier = dynamic_cast<MapViewItemsNotifier*>(ghostItem);
    if(notifier) notifier->disableNotifications();

    return ghostItem;
}

void ViewMapHint::integrateDrawingAsPayload(QGraphicsPathItem* drawnItem, QGraphicsItem* templateGhostItem) {
    if(!templateGhostItem) return;

    //from ghost item
    auto newAtom = MapViewGraphicsScene::itemToAtom(templateGhostItem);
    
    //override shape and pos to fit the drawn item
    newAtom.setShape(drawnItem->path());
    newAtom.setPos(drawnItem->scenePos());

    auto payload = AddedPayload(newAtom);
    this->handleAlterationRequest(payload);
}

void ViewMapHint::integrateGraphicsItemAsPayload(QGraphicsItem* ghostItem) {
    if(!ghostItem) return;
    auto newAtom = MapViewGraphicsScene::itemToAtom(ghostItem);
    auto payload = AddedPayload(newAtom);
    this->handleAlterationRequest(payload);
}

/////////////////////////////////
// END Atom insertion helpers //
/////////////////////////////////

/////////////////////////
// Integration handler //
/////////////////////////


QGraphicsItem* ViewMapHint::_buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom) {

    QGraphicsItem* newItem = nullptr;
    auto hasMissingAssetFile = false;
    auto pathToAssetFile = QString();
    auto assetId = atomToBuildFrom.assetId();

    //displayable atoms
    if(!assetId.isEmpty()) {
        assetId = atomToBuildFrom.assetId();
        pathToAssetFile = AssetsDatabase::get()->getFilePathToAsset(assetId);
        hasMissingAssetFile = pathToAssetFile.isEmpty();
    }

    //depending on presence in asset db...
    if(hasMissingAssetFile) {
        
        //add placeholder
        auto placeholder = this->scene()->addMissingAssetPH(atomToBuildFrom);
        newItem = placeholder;

        //if first time the ID is encountered
        if(!this->_missingAssetsIdsFromDb.contains(assetId)) {

            //add graphic item to list of items to replace at times
            this->_missingAssetsIdsFromDb.insert(assetId, placeholder);
            emit requestMissingAsset(assetId);
        }

    } 
    
    //default
    else {
        newItem = this->scene()->addToScene(atomToBuildFrom, MVPayload(pathToAssetFile));
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
        auto newGi = this->scene()->addToScene(*atom, MVPayload(pathToFile));
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

/////////////////////////////
// AtomsStorage Overriding //
/////////////////////////////

//alter Scene
void ViewMapHint::_handlePayload(AlterationPayload* payload) { 

    //make sure to not perpetuate circular payloads
    auto payloadSource = payload->source();
    if(payloadSource == this->_source) return;

    this->_preventInnerGIEventsHandling = true;

    //on reset
    auto type = payload->type();
    if(type == PayloadAlteration::Selected) this->scene()->clearSelection();
    if(type == PayloadAlteration::Reset) {
        for(auto &atom : this->_atomsById) {
            auto gi = atom.graphicsItem();
            if(gi) delete gi;
        }
    }
    
    AtomsStorage::_handlePayload(payload);

    //define dirty
    this->_shouldMakeDirty(payload);

    this->_preventInnerGIEventsHandling = false;
}

//register actions
RPZAtom* ViewMapHint::_handlePayloadInternal(const PayloadAlteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) {
   
    //default handling
    auto updatedAtom = AtomsStorage::_handlePayloadInternal(type, targetedAtomId, atomAlteration); 

    //by alteration
    switch(type) {
        
        //on addition
        case PayloadAlteration::Reset:
        case PayloadAlteration::Added: {
            this->_buildGraphicsItemFromAtom(*updatedAtom);
        }
        break;
        
        //on focus
        case PayloadAlteration::Focused: {
            this->_boundGv->centerOn(updatedAtom->graphicsItem());
        }
        break;

        //on moving
        case PayloadAlteration::Moved: {
            auto destPos = updatedAtom->pos();
            updatedAtom->graphicsItem()->setPos(destPos);  
        }
        break;

        //on scaling
        case PayloadAlteration::Scaled: {
            auto destScale = updatedAtom->scale();
            updatedAtom->graphicsItem()->setScale(destScale);
        }
        break;

        // on locking change
        case PayloadAlteration::LockChanged: {
            auto locked = updatedAtom->isLocked();
            auto flags = !locked ? MapViewItemsNotifier::defaultFlags() : 0;
            updatedAtom->graphicsItem()->setFlags(flags);
        }
        break;
        
        // on changing visibility
        case PayloadAlteration::VisibilityChanged: {
            auto hidden = updatedAtom->isHidden();
            auto opacity = hidden ? .01 : 1;
            updatedAtom->graphicsItem()->setOpacity(opacity);
        }
        break;

        //on rotation
        case PayloadAlteration::Rotated: {
            auto destRotation = updatedAtom->rotation();
            updatedAtom->graphicsItem()->setRotation(destRotation);
        }
        break;

        //on text change
        case PayloadAlteration::TextChanged: {
            auto newText = updatedAtom->text();
            auto cItem = (QGraphicsTextItem*)updatedAtom->graphicsItem();
            cItem->setPlainText(newText);
        }
        break;

        //on layer change
        case PayloadAlteration::LayerChanged: {
            auto newLayer = updatedAtom->layer();
            updatedAtom->graphicsItem()->setZValue(newLayer);
        }
        break;

        //on selection
        case PayloadAlteration::Selected: {
            updatedAtom->graphicsItem()->setSelected(true);
        }
        break;

    }

    return updatedAtom;
}

/////////////////////////////////
// END AtomsStorage Overriding //
/////////////////////////////////