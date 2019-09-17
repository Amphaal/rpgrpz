#include "ViewMapHint.h"

ViewMapHint::ViewMapHint() : AtomsStorage(AlterationPayload::Source::Local_Map) {
    
    //default layer from settings
    this->setDefaultLayer(AppContext::settings()->defaultLayer());

};

const RPZAtom ViewMapHint::templateAtom() const {
    QMutexLocker m(&this->_m_templateAtom);
    return this->_templateAtom;
}

QGraphicsItem* ViewMapHint::ghostItem() const {
    QMutexLocker m(&this->_m_ghostItem);
    return this->_ghostItem;
}

void ViewMapHint::setDefaultLayer(int layer) {
    
    AtomUpdates updates;
    updates.insert(AtomParameter::Layer, layer);

    {
        QMutexLocker m(&this->_m_templateAtom);
        this->_templateAtom.setMetadata(updates);
    }

    this->_m_ghostItem.lock();
    auto ghostItem = this->_ghostItem;
    this->_m_ghostItem.unlock();

    if(ghostItem) {
        emit requestingUIUpdate({ghostItem}, updates);
    }
   
}

void ViewMapHint::notifyFocusedItem(QGraphicsItem* focusedItem) {
    auto cAtom = this->getAtomFromGraphicsItem(focusedItem);
    FocusedPayload payload(cAtom->id());
    AlterationHandler::get()->queueAlteration(this, payload);
}

               
void ViewMapHint::mightNotifyMovement(const QList<QGraphicsItem*> &itemsWhoMightHaveMoved) {

    //generate args for payload
    AtomsUpdates coords;
    for(auto &gi : itemsWhoMightHaveMoved) {
        
        //cannot fetch atom
        auto cAtom = this->getAtomFromGraphicsItem(gi);
        if(!cAtom) continue;
        
        //pos have not changed
        auto newPos = gi->pos();
        if(cAtom->pos() == newPos) continue;

        //add update into alteration
        AtomUpdates updates {{ AtomParameter::Position, newPos }};
        coords.insert(cAtom->id(), updates);

    }

    //if nothing happened
    if(!coords.count()) return;

    //inform moving
    BulkMetadataChangedPayload payload(coords);
    AlterationHandler::get()->queueAlteration(this, payload);
    
}

void ViewMapHint::notifySelectedItems(const QList<QGraphicsItem*> &selectedItems) {

    QVector<RPZAtomId> ids;

    for(auto atom : this->getAtomsFromGraphicsItems(selectedItems)) {
        ids.append(atom->id());
    }

    SelectedPayload payload(ids);
    AlterationHandler::get()->queueAlteration(this, payload);
}

//////////////////
// Pen handling //
//////////////////


void ViewMapHint::setDefaultUser(const RPZUser &user) {
    
    this->_bindDefaultOwner(user);
    
    {
        QMutexLocker m(&this->_m_templateAtom);
        this->_templateAtom.setOwnership(user); //update template
    }

    this->_m_ghostItem.lock();
    auto ghostItem = this->_ghostItem;
    this->_m_ghostItem.unlock();

    if(ghostItem) {
        emit requestingUIUserChange({ghostItem}, user);
    }

}

//////////////////////
// END Pen handling //
//////////////////////

/////////////////////////////
// Atom insertion helpers //
/////////////////////////////

void ViewMapHint::deleteCurrentSelectionItems() const {
    RemovedPayload payload(this->bufferedSelectedAtomIds());
    AlterationHandler::get()->queueAlteration(this, payload);
}

QGraphicsItem* ViewMapHint::_generateGhostItem(const RPZToyMetadata &assetMetadata) {
    
    if(!assetMetadata.isEmpty()){
        QMutexLocker m(&this->_m_templateAtom);

        //update template
        this->_templateAtom.changeType(assetMetadata.atomType());

        this->_templateAtom.setMetadata(AtomParameter::ShapeCenter, assetMetadata.center());
        
        auto sSize = assetMetadata.shapeSize();
        if(!sSize.isEmpty()) {
            this->_templateAtom.setShape(
                QRectF(QPointF(), sSize)
            );
        } else {
            this->_templateAtom.setShape(QRectF()); 
        }

        this->_templateAtom.setMetadata(AtomParameter::AssetId, assetMetadata.assetId());
        this->_templateAtom.setMetadata(AtomParameter::AssetName, assetMetadata.assetName());
    }
    
    QGraphicsItem* toDelete = nullptr;

    {
        QMutexLocker m(&this->_m_ghostItem);

        //delete if ghost item exist
        if(this->_ghostItem) toDelete = this->_ghostItem;

        if(!assetMetadata.isEmpty()) {
            
            {
                QMutexLocker l2(&this->_m_templateAsset);
                this->_templateAsset = assetMetadata;
            }
         
            //add to scene
            this->_ghostItem = generateTemporaryItemFromTemplateBuffer();
            
        } else this->_ghostItem = nullptr;
        
    }

    return toDelete;
}

QGraphicsItem* ViewMapHint::generateTemporaryItemFromTemplateBuffer() {
    QMutexLocker l1(&this->_m_templateAtom);
    QMutexLocker l2(&this->_m_templateAsset);

    return CustomGraphicsItemHelper::createGraphicsItem(
                this->_templateAtom, 
                this->_templateAsset, 
                true
            );
}

void ViewMapHint::integrateGraphicsItemAsPayload(QGraphicsItem* graphicsItem) const {
    if(!graphicsItem) return;
    
    QMutexLocker l1(&this->_m_templateAtom);

    //from ghost item / temporary drawing
    auto newAtom = AtomConverter::graphicsToAtom(graphicsItem, this->_templateAtom);
    AddedPayload payload(newAtom);

    AlterationHandler::get()->queueAlteration(this, payload);
}

/////////////////////////////////
// END Atom insertion helpers //
/////////////////////////////////

/////////////////////////
// Integration handler //
/////////////////////////


QGraphicsItem* ViewMapHint::_buildGraphicsItemFromAtom(const RPZAtom &atomToBuildFrom) {

    QGraphicsItem* newItem = nullptr;
    auto assetId = atomToBuildFrom.assetId();
    auto assetMetadata = AssetsDatabase::get()->getAssetMetadata(assetId);
    auto hasMissingAsset = !assetId.isEmpty() && assetMetadata.pathToAssetFile().isEmpty();

    //atom links to missing asset from DB
    if(hasMissingAsset) {
        
        //add placeholder
        auto placeholder = CustomGraphicsItemHelper::createMissingAssetPlaceholderItem(atomToBuildFrom);
        newItem = placeholder;

        {
            QMutexLocker l(&this->_m_missingAssetsIdsFromDb);

            //add graphic item to list of items to replace at times
            this->_missingAssetsIdsFromDb.insert(assetId, placeholder);
        }

        //if first time the ID is encountered
        if(!this->_assetsIdsToRequest.contains(assetId)) {
            this->_assetsIdsToRequest.insert(assetId);
        }

    } 
    
    //default
    else {
        newItem = CustomGraphicsItemHelper::createGraphicsItem(
            atomToBuildFrom, 
            assetMetadata
        );
    }

    //save pointer ref
    this->_crossBindingAtomWithGI((RPZAtom*)&atomToBuildFrom, newItem);

    return newItem;
}

void ViewMapHint::_replaceMissingAssetPlaceholders(const RPZToyMetadata &metadata) {
    
    QList<QGraphicsItem*> newGis;
    QSet<QGraphicsItem *> setOfGraphicsItemsToReplace;

    {
        QMutexLocker l(&this->_m_missingAssetsIdsFromDb);

        auto assetId = metadata.assetId();
        auto pathToFile = metadata.pathToAssetFile();

        if(!this->_missingAssetsIdsFromDb.contains(assetId)) return; //no assetId, skip
        if(pathToFile.isNull()) return; //path to file empty, skip
        
        //iterate through the list of GI to replace
        setOfGraphicsItemsToReplace = this->_missingAssetsIdsFromDb.values(assetId).toSet();

        for(auto oldGi : setOfGraphicsItemsToReplace) {
            
            //find corresponding atom
            auto atom = this->getAtomFromGraphicsItem(oldGi);

            //create the new graphics item
            auto newGi = CustomGraphicsItemHelper::createGraphicsItem(*atom, metadata);
            this->_crossBindingAtomWithGI(atom, newGi);
            newGis.append(newGi);

        }

        //clear the id from the missing list
        this->_missingAssetsIdsFromDb.remove(assetId);
    }

    //remove old
    emit requestingUIAlteration(PA_Removed, setOfGraphicsItemsToReplace.toList());

    //replace by new
    emit requestingUIAlteration(PA_Added, newGis);
}

void ViewMapHint::handlePreviewRequest(const AtomsSelectionDescriptor &selectionDescriptor, const AtomParameter &parameter, const QVariant &value) {
    
    //create updates container
    AtomUpdates updates; updates.insert(parameter, value);
    
    QList<QGraphicsItem*> toUpdate;

    //is ghost that must be targeted
    if(!selectionDescriptor.selectedAtomIds.count() && !selectionDescriptor.templateAtom.isEmpty()) {
        
        QMutexLocker l(&this->_m_ghostItem);

        if(this->_ghostItem) toUpdate += this->_ghostItem;

    }

    //selected atoms to change
    else {
        QMutexLocker l(&this->_m_GItemsByRPZAtomId);

        for(auto &id : selectionDescriptor.selectedAtomIds) {
            toUpdate += this->_GItemsByRPZAtomId[id];
        }
    }

    emit requestingUIUpdate(toUpdate, updates);
}


/////////////////////////////
// END Integration handler //
/////////////////////////////

//////////////////////
// Internal Helpers //
//////////////////////

void ViewMapHint::_crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi) {
    this->_GItemsByRPZAtomId[atom->id()] = gi;
    gi->setData(RPZUserRoles::AtomPtr, QVariant::fromValue<RPZAtom*>(atom));
}

QVector<RPZAtom*> ViewMapHint::getAtomsFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const {
    QVector<RPZAtom*> list;
    for(auto e : listToFetch) {
        auto atom = this->getAtomFromGraphicsItem(e);
        list.append(atom);
    }
    return list;
}

RPZAtom* ViewMapHint::getAtomFromGraphicsItem(QGraphicsItem* graphicElem) const {
    auto ptr = graphicElem->data(RPZUserRoles::AtomPtr).value<RPZAtom*>();
    if(!ptr) qWarning() << "No atom assigned to this graphics item...";
    auto q = ptr;
    return ptr;
}

//////////////////////////
// END Internal Helpers //
//////////////////////////

/////////////////////////////
// AtomsStorage Overriding //
/////////////////////////////

//alter Scene
void ViewMapHint::_handleAlterationRequest(AlterationPayload &payload) {
    
    {
        QMutexLocker l(&this->_m_GItemsByRPZAtomId);
        AtomsStorage::_handleAlterationRequest(payload);
    }

    //if reset, delete ghost
    if(auto mPayload = dynamic_cast<ResetPayload*>(&payload)) {
        QMutexLocker l(&this->_m_ghostItem);
        this->_ghostItem = nullptr;
    }

    //if asset changed
    if(auto mPayload = dynamic_cast<AssetChangedPayload*>(&payload)) {
        this->_replaceMissingAssetPlaceholders(mPayload->assetMetadata());
    }

    //if asset selected
    else if(auto mPayload = dynamic_cast<AssetSelectedPayload*>(&payload)) {
        
        //generate ghost
        auto mightDelete = this->_generateGhostItem(mPayload->selectedAsset());
        
        //request deletion previous ghost
        if(mightDelete) {
            emit requestingUIAlteration(PayloadAlteration::PA_Removed, {mightDelete});
        }

        this->_m_ghostItem.lock();
        auto ghostItem = this->_ghostItem;
        this->_m_ghostItem.unlock();

        //request addition of new ghost
        emit requestingUIAlteration(PayloadAlteration::PA_AssetSelected, {ghostItem});
    }
    
    //if template changed
    else if(auto mPayload = dynamic_cast<AtomTemplateChangedPayload*>(&payload)) {

        auto updates = mPayload->updates();

        {
            QMutexLocker m(&this->_m_templateAtom);
            this->_templateAtom.setMetadata(updates);
        }

        this->_m_ghostItem.lock();
        auto ghostItem = this->_ghostItem;
        this->_m_ghostItem.unlock();

        if(ghostItem) {
            //says it changed
            emit requestingUIUpdate({ghostItem}, updates);
        }
        
    }

    //request assets if there are missing
    auto c_MissingAssets = this->_assetsIdsToRequest.count();
    if(c_MissingAssets) {
        qDebug() << "Assets : missing" << QString::number(c_MissingAssets).toStdString().c_str() << "asset(s)" << this->_assetsIdsToRequest.toList();
        auto toRequest = this->_assetsIdsToRequest.toList();
        this->_assetsIdsToRequest.clear();
        emit requestMissingAssets(toRequest);
    }

}

RPZAtom* ViewMapHint::_insertAtom(const RPZAtom &newAtom) {
    auto updatedAtom = AtomsStorage::_insertAtom(newAtom);
    this->_buildGraphicsItemFromAtom(*updatedAtom);
    return updatedAtom;
}

RPZAtom* ViewMapHint::_changeOwner(RPZAtom* atomWithNewOwner, const RPZUser &newOwner) {
    auto updatedAtom = AtomsStorage::_changeOwner(atomWithNewOwner, newOwner);
    
    //filter by determining if is a drawing type if it must be altered
    if(updatedAtom->type() != AtomType::Drawing) {
        return nullptr;
    }

    return updatedAtom;
}

void ViewMapHint::_basicAlterationDone(const QList<RPZAtomId> &updatedIds, const PayloadAlteration &type) {
    QList<QGraphicsItem*> toUpdate;
    for(auto id : updatedIds) {
        if(type == PA_Removed) toUpdate += this->_GItemsByRPZAtomId.take(id);
        else toUpdate += this->_GItemsByRPZAtomId[id];
    }
    emit requestingUIAlteration(type, toUpdate);
}

void ViewMapHint::_updatesDone(const QList<RPZAtomId> &updatedIds, const AtomUpdates &updates) {
    QList<QGraphicsItem*> toUpdate;
    for(auto id : updatedIds) {
        toUpdate += this->_GItemsByRPZAtomId[id];
    }
    emit requestingUIUpdate(toUpdate, updates);
}

void ViewMapHint::_updatesDone(const AtomsUpdates &updates) {
    QHash<QGraphicsItem*, AtomUpdates> toUpdate;
    for(auto i = updates.constBegin(); i != updates.constEnd(); i++) {
        auto gi = this->_GItemsByRPZAtomId[i.key()];
        toUpdate.insert(gi, i.value());
    }
    emit requestingUIUpdate(toUpdate);
}

void ViewMapHint::_ownerChangeDone(const QList<RPZAtomId> &updatedIds, const RPZUser &newUser) {
    QList<QGraphicsItem*> toUpdate;
    for(auto id : updatedIds) {
        toUpdate += this->_GItemsByRPZAtomId[id];
    }
    emit requestingUIUserChange(toUpdate, newUser);
}

/////////////////////////////////
// END AtomsStorage Overriding //
/////////////////////////////////