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


void ViewMapHint::setDefaultUser(const RPZUser &user) {
    this->_bindDefaultOwner(user);
    this->templateAtom->setOwnership(user); //update template
    emit atomTemplateChanged();
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
    QHash<snowflake_uid, QGraphicsItem*> oldB;
    QHash<snowflake_uid, QGraphicsItem*> newB;
    QList<RPZAtom> toRebuild;

    for(auto oldGi : setOfGraphicsItemsToReplace) {
        
        //find corresponding atom
        auto atom = this->_getAtomFromGraphicsItem(oldGi);
        toRebuild.append(*atom);

        //create the new graphics item
        auto newGi = CustomGraphicsItemHelper::createGraphicsItem(*atom, metadata);
        this->_crossBindingAtomWithGI(atom, newGi);

        oldB.insert(atom->id(), oldGi);
        newB.insert(atom->id(), newGi);

    }

    //clear the id from the missing list
    this->_missingAssetsIdsFromDb.remove(assetId);

    //remove old
    RemovedPayload rPayload(oldB.keys().toVector());
    emit requestingUIAlteration(rPayload, oldB);

    //replace by new
    AddedPayload aPayload(toRebuild);
    emit requestingUIAlteration(aPayload, newB);
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
        auto updates = mtPayload->updates();
        
        for(auto i = updates.begin(); i != updates.end(); i++) {
            this->templateAtom->setMetadata(i.key(), i.value());
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
    emit requestingUIAlteration(payload, this->_UIUpdatesBuffer);

}

//register actions
RPZAtom* ViewMapHint::_handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) {
   
    //default handling
    auto updatedAtom = AtomsStorage::_handlePayloadInternal(type, targetedAtomId, alteration); 
    
    QGraphicsItem* graphicsItem = nullptr;

    //by alteration
    switch(type) {
        
        //on addition
        case PA_Reset:
        case PA_Added: {
            graphicsItem = this->_buildGraphicsItemFromAtom(*updatedAtom);
        }
        break;
        
        //on deletion
        case PA_Removed: {
            graphicsItem = this->_GItemsByAtomId.take(targetedAtomId);
        }
        break;

        //on owner change
        case PA_OwnerChanged: {
            
            //filter by determining if is a drawing type
            if(updatedAtom->type() == AtomType::Drawing) {
                graphicsItem = this->_GItemsByAtomId[targetedAtomId];
            }

        };

        default: {
            graphicsItem = this->_GItemsByAtomId[targetedAtomId];
        }
        break;

    }

    //update buffers for UI event emission
    if(!graphicsItem) this->_UIUpdatesBuffer.insert(targetedAtomId, graphicsItem);

    return updatedAtom;
}

/////////////////////////////////
// END AtomsStorage Overriding //
/////////////////////////////////