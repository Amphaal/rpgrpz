#include "ViewMapHint.h"

ViewMapHint::ViewMapHint() : AtomsStorage(AlterationPayload::Source::Local_Map),
    templateAtom(new RPZAtom) {
    
    //default layer from settings
    this->setDefaultLayer(AppContext::settings()->defaultLayer());

};

void ViewMapHint::setDefaultLayer(int layer) {
    this->templateAtom->setMetadata(AtomParameter::Layer, layer);
    emit atomTemplateChanged();
}

void ViewMapHint::notifyMovementOnItems(QList<QGraphicsItem*> &itemsWhoMoved) {

    //generate args for payload
    RPZMap<RPZAtom> coords;
    for(auto &gi : itemsWhoMoved) {
        
        auto cAtom = this->_getAtomFromGraphicsItem(gi);
        if(!cAtom) continue;

        RPZAtom oAtom;
        oAtom.setMetadata(AtomParameter::Position, gi->pos());
        coords.insert(cAtom->id(), oAtom);

    }

    //inform moving
    BulkMetadataChangedPayload payload(coords);
    AlterationHandler::get()->queueAlteration(this, payload);
    
}

QVector<RPZAtom*> ViewMapHint::selectedAtoms() {
    QVector<RPZAtom*> out;
    for(const auto &selectedId : this->selectedAtomIds()) {
        auto &atom = this->_atomsById[selectedId];
        out.append(&atom);
    }
    return out;
}

void ViewMapHint::notifySelectedItems(QList<QGraphicsItem*> &selectedItems) {

    QVector<snowflake_uid> ids;

    for(auto &gi : selectedItems) {
        
        auto cAtom = this->_getAtomFromGraphicsItem(gi);
        if(!cAtom) continue;

        ids.append(cAtom->id());

    }

    SelectedPayload payload(ids);
    AlterationHandler::get()->queueAlteration(this, payload);
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
        auto gi = this->_GItemsByAtomId[elemId];
        
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
    AlterationHandler::get()->queueAlteration(this, payload);

}

//////////////////////
// END Pen handling //
//////////////////////

/////////////////////////////
// Atom insertion helpers //
/////////////////////////////

void ViewMapHint::deleteCurrentSelectionItems() {
    RemovedPayload payload(this->selectedAtomIds());
    AlterationHandler::get()->queueAlteration(this, payload);
}

QGraphicsItem* ViewMapHint::generateGhostItem(RPZAssetMetadata &assetMetadata) {

    //update template
    this->templateAtom->changeType(assetMetadata.atomType());
    this->templateAtom->setMetadata(AtomParameter::AssetId, assetMetadata.assetId());
    this->templateAtom->setMetadata(AtomParameter::AssetName, assetMetadata.assetName());
    
    //generate a blueprint
    auto atomBuiltFromTemplate = RPZAtom(*this->templateAtom);

    //add to scene
    QGraphicsItem* ghostItem = CustomGraphicsItemHelper::createGraphicsItem(atomBuiltFromTemplate, assetMetadata, true);

    //advert change in template
    emit atomTemplateChanged();

    //define transparency as it is a dummy
    ghostItem->setOpacity(.5);

    return ghostItem;
}

void ViewMapHint::integrateGraphicsItemAsPayload(QGraphicsItem* graphicsItem) {
    if(!graphicsItem) return;
    
    //from ghost item / temporary drawing
    auto newAtom = AtomConverter::graphicsToAtom(graphicsItem);
    AddedPayload payload(newAtom);

    AlterationHandler::get()->queueAlteration(this, payload);
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
        auto placeholder = CustomGraphicsItemHelper::createMissingAssetPlaceholderItem(atomToBuildFrom);
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
        newItem = CustomGraphicsItemHelper::createGraphicsItem(
            atomToBuildFrom, 
            metadata
        );
    }

    //save pointer ref
    this->_crossBindingAtomWithGI(&atomToBuildFrom, newItem);

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
        auto newGi = CustomGraphicsItemHelper::createGraphicsItem(*atom, metadata);
        this->_crossBindingAtomWithGI(atom, newGi);
    }

    //clear the id from the missing list
    this->_missingAssetsIdsFromDb.remove(assetId);

    //remove old
    emit requestingUIAlteration(PA_Removed, setOfGraphicsItemsToReplace.toList());
}

void ViewMapHint::handlePreviewRequest(const QVector<snowflake_uid> &atomIdsToPreview, const AtomParameter &parameter, QVariant &value) {
    for(const auto &id : atomIdsToPreview) {
        AtomConverter::updateGraphicsItemFromMetadata(this->_GItemsByAtomId[id], parameter, value);
    }
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
        AlterationHandler::get()->queueAlteration(this, *cPayload);

    }
}

/////////////////////////////
// END Integration handler //
/////////////////////////////

//////////////////////
// Internal Helpers //
//////////////////////

void ViewMapHint::_crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi) {
    this->_GItemsByAtomId[atom->id()] = gi;
    auto ptrValToAtom = (long long)atom;
    gi->setData(RPZUserRoles::AtomPtr, ptrValToAtom);
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
void ViewMapHint::_handleAlterationRequest(AlterationPayload &payload) { 
    
    //on reset
    auto type = payload.type();

    if(type == PayloadAlteration::PA_Reset) emit heavyAlterationProcessing();
    
    this->_UIUpdatesBuffer.clear();

    AtomsStorage::_handleAlterationRequest(payload);

    //request assets if there are missing
    auto c_MissingAssets = this->_assetsIdsToRequest.count();
    if(c_MissingAssets) {
        qDebug() << "Assets : missing" << QString::number(c_MissingAssets).toStdString().c_str() << "asset(s)" << this->_assetsIdsToRequest.toList();
        auto toRequest = this->_assetsIdsToRequest.toList();
        this->_assetsIdsToRequest.clear();
        emit requestMissingAssets(toRequest);
    }

    //send UI events
    if(type ==PA_MetadataChanged || type == PA_BulkMetadataChanged) {
        emit requestingUIUpdate(type, this->_UIUpdatesBuffer);
    } else {
        emit requestingUIAlteration(type, this->_UIUpdatesBuffer.keys());
    }

}

//register actions
RPZAtom* ViewMapHint::_handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) {
   
    //default handling
    auto updatedAtom = AtomsStorage::_handlePayloadInternal(type, targetedAtomId, alteration); 
    
    QGraphicsItem* graphicsItem = nullptr;
    QHash<AtomParameter, QVariant> maybeNewData;

    //by alteration
    switch(type) {
        
        //on addition
        case PayloadAlteration::PA_Reset:
        case PayloadAlteration::PA_Added: {
            graphicsItem = this->_buildGraphicsItemFromAtom(*updatedAtom);
        }
        break;
        
        //on deletion
        case PayloadAlteration::PA_Removed: {
            graphicsItem = this->_GItemsByAtomId.take(targetedAtomId);
        }
        break;

        case PayloadAlteration::PA_MetadataChanged:
        case PayloadAlteration::PA_BulkMetadataChanged: {

            auto partial = type == PayloadAlteration::PA_BulkMetadataChanged ? 
                                    RPZAtom(alteration.toHash()) : 
                                    MetadataChangedPayload::fromArgs(alteration);
            
            for(auto param : partial.editedMetadata()) {
                auto paramVal = updatedAtom->metadata(param);
                maybeNewData.insert(param, paramVal);
            }

        }   
        break;

        default:
            break;

    }

    //get graphics item as default
    if(!graphicsItem) graphicsItem = this->_GItemsByAtomId[targetedAtomId];

    //update buffers for UI event emission
    this->_UIUpdatesBuffer.insert(graphicsItem, maybeNewData);

    return updatedAtom;
}

/////////////////////////////////
// END AtomsStorage Overriding //
/////////////////////////////////