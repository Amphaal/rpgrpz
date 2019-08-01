#include "ViewMapHint.h"

ViewMapHint::ViewMapHint(QGraphicsView* boundGv) : AtomsStorage(AlterationPayload::Source::Local_Map), 
    AtomsContextualMenuHandler(this, boundGv), 
    templateAtom(new RPZAtom),
    _boundGv(boundGv),
    _hiddingBrush(new QBrush("#EEE", Qt::BrushStyle::SolidPattern)) {
    
    //auto hidding
    this->_boundGv->setForegroundBrush(*this->_hiddingBrush);
    
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

void ViewMapHint::setDefaultLayer(int layer) {
    this->templateAtom->setMetadata(AtomParameter::Layer, layer);
    emit atomTemplateChanged();
}

void ViewMapHint::handleAnyMovedItems() {
    
    //if no item moved since last call, do nothing
    if(!this->_itemsWhoNotifiedMovement.count()) return;

    //generate args for payload
    RPZMap<RPZAtom> coords;
    for(auto gi : this->_itemsWhoNotifiedMovement) {
        
        auto cAtom = this->_getAtomFromGraphicsItem(gi);
        if(!cAtom) continue;

        RPZAtom oAtom;
        oAtom.setMetadata(AtomParameter::Position, gi->pos());
        coords.insert(cAtom->id(), oAtom);

    }

    //enable notifications back on those items
    for(auto item : this->_itemsWhoNotifiedMovement) {
        if(auto notifier = dynamic_cast<MapViewItemsNotifier*>(item)) {
            notifier->activateNotifications();
        }
    }

    //reset list 
    this->_itemsWhoNotifiedMovement.clear();

    //inform moving
    BulkMetadataChangedPayload payload(coords);
    this->queueAlteration(payload);
    
}

void ViewMapHint::_onSceneItemChanged(QGraphicsItem* item, int changeFlag) {

    if(this->_preventInnerGIEventsHandling) return;

    switch(changeFlag) {
        case (int)MapViewCustomItemsEventFlag::Moved: {

            //add to list for future information
            this->_itemsWhoNotifiedMovement.insert(item);

            //disable further notifications until information have been handled
            if(auto notifier = dynamic_cast<MapViewItemsNotifier*>(item)) {
                notifier->disableNotifications();
            }

        }
        break;
    }

}


QVector<RPZAtom*> ViewMapHint::selectedAtoms() {
    return this->_getAtomFromGraphicsItems(this->scene()->selectedItems());
}

QVector<snowflake_uid> ViewMapHint::_selectedAtomIds() {
    QVector<snowflake_uid> selectedAtomIds;
    
    for(auto atom : this->selectedAtoms()) {
        selectedAtomIds.append(atom->id());
    }
    
    return selectedAtomIds;
}

void ViewMapHint::_onSceneSelectionChanged() {
    
    //prevent
    if(this->_preventInnerGIEventsHandling) return;

    //bypass internal
    SelectedPayload payload(this->_selectedAtomIds());
    this->propagateAlterationPayload(payload);

}

//////////////////
// Pen handling //
//////////////////


void ViewMapHint::setDefaultUser(RPZUser user) {
    
    //update template
    this->_defaultOwner = user;
    auto oldOwnerId = this->templateAtom->owner().id();
    this->templateAtom->setOwnership(user);
    emit atomTemplateChanged();

    //update self graphic path items with new color
    auto color = user.color();

    auto atomIds = this->_atomIdsByOwnerId[oldOwnerId];
    for(auto &elemId : atomIds) {
        
        auto &atom = this->_atomsById[elemId];
        auto gi = atom.graphicsItem();
        
        //determine if is a drawing type
        if(atom.type() == AtomType::Drawing) {
            if(auto pathItem = dynamic_cast<QGraphicsPathItem*>(gi)) {
                auto c_pen = pathItem->pen();
                c_pen.setColor(color);
                pathItem->setPen(c_pen);
            } 
        }
        
        //redefine ownership
        atom.setOwnership(user);
    }

    //change bound
    this->_atomIdsByOwnerId.remove(oldOwnerId);
    this->_atomIdsByOwnerId.insert(user.id(), atomIds);

    //inform atom layout
    OwnerChangedPayload payload(atomIds.toList().toVector(), user);
    this->propagateAlterationPayload(payload);

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
        auto atom = this->_getAtomFromGraphicsItem(item);
        atomIdsToRemove.append(atom->id());
    }

    RemovedPayload payload(atomIdsToRemove);
    this->queueAlteration(payload);
}

QGraphicsItem* ViewMapHint::generateGhostItem(RPZAssetMetadata &assetMetadata) {

    //update template
    this->templateAtom->changeType(assetMetadata.atomType());
    this->templateAtom->setMetadata(AtomParameter::AssetId, assetMetadata.assetId());
    this->templateAtom->setMetadata(AtomParameter::AssetName, assetMetadata.assetName());
    
    //generate a blueprint
    auto atomBuiltFromTemplate = RPZAtom(*this->templateAtom);

    //add to scene
    QGraphicsItem* ghostItem = this->scene()->addToScene(atomBuiltFromTemplate, assetMetadata, true);
    this->templateAtom->setGraphicsItem(ghostItem);

    //advert change in template
    emit atomTemplateChanged();

    //if no ghost item, return
    if(!ghostItem) return ghostItem;

    //define transparency as it is a dummy
    ghostItem->setOpacity(.5);

    return ghostItem;
}

void ViewMapHint::integrateGraphicsItemAsPayload(QGraphicsItem* graphicsItem) {
    if(!graphicsItem) return;
    
    //from ghost item / temporary drawing
    auto newAtom = AtomConverter::graphicsToAtom(graphicsItem);
    AddedPayload payload(newAtom);

    this->queueAlteration(payload);
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

        //add graphic item to list of items to replace at times
        this->_missingAssetsIdsFromDb.insert(assetId, placeholder);

        //if first time the ID is encountered
        if(!this->_assetsIdsToRequest.contains(assetId)) {
            this->_assetsIdsToRequest.insert(assetId);
        }

    } 
    
    //default
    else {
        auto metadata = RPZAssetMetadata(assetId, pathToAssetFile);
        newItem = this->scene()->addToScene(
            atomToBuildFrom, 
            metadata
        );
    }

    //save pointer ref
    this->_crossBindingAtomWithGI(&atomToBuildFrom, newItem);

    //request insert
    emit requestingItemInsertion(newItem);

    return newItem;
}

void ViewMapHint::replaceMissingAssetPlaceholders(const RPZAssetMetadata &metadata) {
    
    auto assetId = metadata.assetId();
    auto pathToFile = metadata.pathToAssetFile();

    if(!this->_missingAssetsIdsFromDb.contains(assetId)) return; //no assetId, skip
    if(pathToFile.isNull()) return; //path to file empty, skip
    
    //iterate through the list of GI to replace
    auto setOfGraphicsItemsToReplace = this->_missingAssetsIdsFromDb.values(assetId).toSet();
    for(auto gi : setOfGraphicsItemsToReplace) {
        
        //find corresponding atom
        auto atom = this->_getAtomFromGraphicsItem(gi);

        //create the new graphics item
        auto newGi = this->scene()->addToScene(*atom, metadata);
        this->_crossBindingAtomWithGI(atom, newGi);

        delete gi;
    }

    //clear the id from the missing list
    this->_missingAssetsIdsFromDb.remove(assetId);
}

void ViewMapHint::handleParametersUpdateAlterationRequest(QVariantHash &payload) {
    
    auto cPayload = Payloads::autoCast(payload);
    
    auto mtPayload = cPayload.dynamicCast<MetadataChangedPayload>();
    auto targets = mtPayload->targetAtomIds();
    auto firstTargetId = targets.count() > 0 ? targets[0] : 0;

    //if single target and no ID == templateAtom update
    if(!firstTargetId) {

        //update template
        auto partial = MetadataChangedPayload::fromArgs(mtPayload->args());
        
        for(auto param : partial.editedMetadata()) {
            this->templateAtom->setMetadata(param, partial);
        }
        
        //says it changed
        emit atomTemplateChanged();

    } 
    
    else {

        //Change for this source to take ownership at re-emission and prevent circular event
        cPayload->changeSource(AlterationPayload::Source::Undefined); 

        //handle payload
        this->queueAlteration(*cPayload);

    }
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
    gi->setData(RPZUserRoles::AtomPtr, ptrValToAtom);
}

MapViewGraphicsScene* ViewMapHint::scene() {
    return (MapViewGraphicsScene*)this->_boundGv->scene();
}

QVector<RPZAtom*> ViewMapHint::_getAtomFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const {
    QVector<RPZAtom*> list;
    for(auto e : listToFetch) {
        auto atom = this->_getAtomFromGraphicsItem(e);
        list.append(atom);
    }
    return list;
}

RPZAtom* ViewMapHint::_getAtomFromGraphicsItem(QGraphicsItem* graphicElem) const {
    auto ptrValToAtom = graphicElem->data(RPZUserRoles::AtomPtr).toLongLong();
    return (RPZAtom*)ptrValToAtom;
}

//////////////////////////
// END Internal Helpers //
//////////////////////////

/////////////////////////////
// AtomsStorage Overriding //
/////////////////////////////

//alter Scene
void ViewMapHint::_handlePayload(AlterationPayload &payload) { 

    this->_preventInnerGIEventsHandling = true;

    //on reset
    auto type = payload.type();
    if(type == PayloadAlteration::PA_Selected) this->scene()->clearSelection();
    if(type == PayloadAlteration::PA_Reset) emit requestingAllItemsRemoval(); //this->scene()->clear();
    
    AtomsStorage::_handlePayload(payload);

    this->_preventInnerGIEventsHandling = false;

    //request assets if there are missing
    auto c_MissingAssets = this->_assetsIdsToRequest.count();
    if(c_MissingAssets) {
        qDebug() << "Assets : missings " << QString::number(c_MissingAssets) << "asset(s)" << this->_assetsIdsToRequest.toList();
        auto toRequest = this->_assetsIdsToRequest.toList();
        this->_assetsIdsToRequest.clear();
        emit requestMissingAssets(toRequest);
    }

}

//register actions
RPZAtom* ViewMapHint::_handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) {
   
    //default handling
    auto updatedAtom = AtomsStorage::_handlePayloadInternal(type, targetedAtomId, alteration); 

    //by alteration
    switch(type) {
        
        //on addition
        case PayloadAlteration::PA_Reset:
        case PayloadAlteration::PA_Added: {
            this->_buildGraphicsItemFromAtom(*updatedAtom);
        }
        break;
        
        //on focus
        case PayloadAlteration::PA_Focused: {
            this->_boundGv->centerOn(updatedAtom->graphicsItem());
        }
        break;

        case PayloadAlteration::PA_MetadataChanged:
        case PayloadAlteration::PA_BulkMetadataChanged: {

            auto partial = type == PayloadAlteration::PA_BulkMetadataChanged ? RPZAtom(alteration.toHash()) : MetadataChangedPayload::fromArgs(alteration);
            for(auto param : partial.editedMetadata()) {
                
                auto paramVal = updatedAtom->metadata(param);
                AtomConverter::updateGraphicsItemFromMetadata(
                    updatedAtom->graphicsItem(),
                    param,
                    paramVal
                );
            
            }
        }   
        break;


        //on selection
        case PayloadAlteration::PA_Selected: {
            updatedAtom->graphicsItem()->setSelected(true);
        }
        break;

        default:
            break;

    }

    return updatedAtom;
}

/////////////////////////////////
// END AtomsStorage Overriding //
/////////////////////////////////