#include "ViewMapHint.h"

ViewMapHint::ViewMapHint() : AtomsStorage(Payload::Source::Local_Map) {
    
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
    
    //extract id
    auto id = this->getAtomIdFromGraphicsItem(focusedItem);
    if(!id) return;

    //send payload
    FocusedPayload payload(id);
    AlterationHandler::get()->queueAlteration(this, payload);

}

               
void ViewMapHint::mightNotifyMovement(const QList<QGraphicsItem*> &itemsWhoMightHaveMoved) {
    
    //generate args for payload
    AtomsUpdates coords;
    for(auto gi : itemsWhoMightHaveMoved) {
        
        //find id
        auto id = this->getAtomIdFromGraphicsItem(gi);
        if(!id) continue;

        //find corresponding atom
        auto cAtom = this->_map.atom(id);
        if(!cAtom) continue;
        
        //pos have not changed
        auto newPos = gi->pos();
        auto oldPos = cAtom->pos();
        if(oldPos == newPos) continue;

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
    auto ids = this->getAtomIdsFromGraphicsItems(selectedItems); //allow selecting nothing
    SelectedPayload payload(ids);
    AlterationHandler::get()->queueAlteration(this, payload);
}

/////////////////////////////
// Atom insertion helpers //
/////////////////////////////

QGraphicsItem* ViewMapHint::_generateGhostItem(const RPZToy &toy) {

    if(!toy.isEmpty()){
        QMutexLocker m(&this->_m_templateAtom);

        //update template
        this->_templateAtom.changeType(toy.atomType());

        this->_templateAtom.setMetadata(AtomParameter::ShapeCenter, toy.shapeCenter());
        
        auto sSize = toy.shape();
        if(!sSize.isEmpty()) {
            this->_templateAtom.setShape(
                QRectF(QPointF(), sSize)
            );
        } else {
            this->_templateAtom.setShape(QRectF()); 
        }

        this->_templateAtom.setMetadata(AtomParameter::AssetHash, toy.hash());
        this->_templateAtom.setMetadata(AtomParameter::AssetName, toy.name());
    }
    
    QGraphicsItem* toDelete = nullptr;

    {
        QMutexLocker m(&this->_m_ghostItem);

        //delete if ghost item exist
        if(this->_ghostItem) toDelete = this->_ghostItem;

        if(!toy.isEmpty()) {
            
            {
                QMutexLocker l2(&this->_m_templateAsset);
                this->_templateToy = toy;
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
        this->_templateToy, 
        true
    );
}

RPZAtomId ViewMapHint::integrateGraphicsItemAsPayload(QGraphicsItem* graphicsItem) const {
    
    if(!graphicsItem) return 0;
    
    //from ghost item / temporary drawing
    RPZAtom newAtom;
    {
        QMutexLocker l1(&this->_m_templateAtom);
        newAtom = AtomConverter::graphicsToAtom(graphicsItem, this->_templateAtom);
    }

    //queue
    AddedPayload payload(newAtom);
    AlterationHandler::get()->queueAlteration(this, payload);

    return newAtom.id();
    
}

/////////////////////////////////
// END Atom insertion helpers //
/////////////////////////////////

/////////////////////////
// Integration handler //
/////////////////////////


QGraphicsItem* ViewMapHint::_buildGraphicsItemFromAtom(const RPZAtom &atomToBuildFrom) {

    QGraphicsItem* newItem = nullptr;
    auto assetHash = atomToBuildFrom.assetHash();
    auto asset = AssetsDatabase::get()->asset(atomToBuildFrom.assetHash());
    auto hasMissingAsset = !assetHash.isEmpty() && !asset;

    //atom links to missing asset from DB
    if(hasMissingAsset) {
        
        //add placeholder
        auto placeholder = CustomGraphicsItemHelper::createMissingAssetPlaceholderItem(atomToBuildFrom);
        newItem = placeholder;

        //add graphic item to list of items to replace at times
        this->_missingAssetHashesFromDb.insert(assetHash, placeholder);

    } 
    
    //default
    else {
        newItem = CustomGraphicsItemHelper::createGraphicsItem(
            atomToBuildFrom, 
            *asset
        );
    }

    //save pointer ref
    this->_crossBindingAtomWithGI(
        atomToBuildFrom, 
        newItem
    );

    return newItem;
}

void ViewMapHint::_replaceMissingAssetPlaceholders(const RPZAsset &metadata) {
    
    QList<QGraphicsItem*> newGis;
    QSet<QGraphicsItem*> setOfGraphicsItemsToReplace;

    auto hash = metadata.hash();
    auto pathToFile = metadata.filepath();

    if(!this->_missingAssetHashesFromDb.contains(hash)) return; //no assetHash, skip
    if(pathToFile.isNull()) return; //path to file empty, skip
    
    //get uniques ids
    setOfGraphicsItemsToReplace = this->_missingAssetHashesFromDb.values(hash).toSet();
    
    //iterate through the list of GI to replace
    for(auto item : setOfGraphicsItemsToReplace) {
        
        //find id
        auto id = this->getAtomIdFromGraphicsItem(item);
        if(!id) continue;

        //find corresponding atom
        auto atom = this->_map.atom(id);
        if(!atom) continue;

        //create the new graphics item
        auto newGi = CustomGraphicsItemHelper::createGraphicsItem(*atom, metadata);
        this->_crossBindingAtomWithGI(*atom, newGi);
        newGis.append(newGi);

    }

    //clear the id from the missing list
    this->_missingAssetHashesFromDb.remove(hash);

    //remove old
    emit requestingUIAlteration(Payload::Alteration::Removed, setOfGraphicsItemsToReplace.toList());

    //replace by new
    emit requestingUIAlteration(Payload::Alteration::Added, newGis);
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
            toUpdate += this->_GItemsByRPZAtomId.value(id);
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

void ViewMapHint::_crossBindingAtomWithGI(const RPZAtom &atom, QGraphicsItem* gi) {
    auto id = atom.id();
    this->_GItemsByRPZAtomId.insert(id, gi);
    RPZQVariant::setAtomId(gi, id);
}

const QVector<RPZAtomId> ViewMapHint::getAtomIdsFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const {
    
    QVector<RPZAtomId> list;

    for(auto e : listToFetch) {
        auto id = this->getAtomIdFromGraphicsItem(e);
        if(id) list += id;
    }

    return list;

}

const RPZAtomId ViewMapHint::getAtomIdFromGraphicsItem(const QGraphicsItem* toFetch) const {
    
    if(!toFetch) {
        qWarning() << "Cannot fetch Atom Id from this non-existant GraphicsItem...";
        return 0;
    }

    auto id = RPZQVariant::atomId(toFetch);
    if(!id) qWarning() << "No atom assigned to this graphics item...";

    return id;

}

//////////////////////////
// END Internal Helpers //
//////////////////////////

/////////////////////////////
// AtomsStorage Overriding //
/////////////////////////////

//alter Scene
void ViewMapHint::_handleAlterationRequest(const AlterationPayload &payload) {

    //if reset (before)
    if(auto mPayload = dynamic_cast<const ResetPayload*>(&payload)) {
        
        //clear cache of missing qGraphicsItem
        this->_missingAssetHashesFromDb.clear();

        //delete ghost
        QMutexLocker l(&this->_m_ghostItem);
        this->_ghostItem = nullptr;

    }

    //standard handling
    {
        QMutexLocker l(&this->_m_GItemsByRPZAtomId);
        AtomsStorage::_handleAlterationRequest(payload);
    }

    //if reset (afterward)
    if(auto mPayload = dynamic_cast<const ResetPayload*>(&payload)) {
        
        //tell UI that download ended
        QMetaObject::invokeMethod(ProgressTracker::get(), "downloadHasEnded", 
            Q_ARG(ProgressTracker::Kind, ProgressTracker::Kind::Map)
        );

    }

    //if asset changed
    if(auto mPayload = dynamic_cast<const AssetChangedPayload*>(&payload)) {
        this->_replaceMissingAssetPlaceholders(mPayload->assetMetadata());
    }

    //if asset selected
    else if(auto mPayload = dynamic_cast<const ToySelectedPayload*>(&payload)) {
        
        //generate ghost
        auto mightDelete = this->_generateGhostItem(mPayload->selectedToy());
        
        //request deletion previous ghost
        if(mightDelete) {
            emit requestingUIAlteration(Payload::Alteration::Removed, {mightDelete});
        }

        this->_m_ghostItem.lock();
        auto ghostItem = this->_ghostItem;
        this->_m_ghostItem.unlock();

        //request addition of new ghost
        emit requestingUIAlteration(Payload::Alteration::ToySelected, {ghostItem});
    }
    
    //if template changed
    else if(auto mPayload = dynamic_cast<const AtomTemplateChangedPayload*>(&payload)) {

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

}

void ViewMapHint::_atomAdded(const RPZAtom &added) {
    this->_buildGraphicsItemFromAtom(added);
}

void ViewMapHint::_basicAlterationDone(const QList<RPZAtomId> &updatedIds, const Payload::Alteration &type) {
    QList<QGraphicsItem*> toUpdate;
    
    for(auto id : updatedIds) {
        if(type == Payload::Alteration::Removed) toUpdate += this->_GItemsByRPZAtomId.take(id);
        else toUpdate += this->_GItemsByRPZAtomId.value(id);
    }

    emit requestingUIAlteration(type, toUpdate);
}

void ViewMapHint::_updatesDone(const QList<RPZAtomId> &updatedIds, const AtomUpdates &updates) {
    QList<QGraphicsItem*> toUpdate;
    for(auto id : updatedIds) {
        toUpdate += this->_GItemsByRPZAtomId.value(id);
    }
    emit requestingUIUpdate(toUpdate, updates);
}

void ViewMapHint::_updatesDone(const AtomsUpdates &updates) {
    QHash<QGraphicsItem*, AtomUpdates> toUpdate;
    for(auto i = updates.constBegin(); i != updates.constEnd(); i++) {
        auto gi = this->_GItemsByRPZAtomId.value(i.key());
        toUpdate.insert(gi, i.value());
    }
    emit requestingUIUpdate(toUpdate);
}

/////////////////////////////////
// END AtomsStorage Overriding //
/////////////////////////////////