#include "AtomsStorage.h"


AtomsStorage::AtomsStorage(const AlterationPayload::Source &boundSource, bool autoLinkage) : AlterationAcknoledger(boundSource, autoLinkage) { };

RPZMap<RPZAtom> AtomsStorage::atoms() const {
    QMutexLocker m(&this->_m_handlingLock);
    return this->_atomsById;
}

QVector<RPZAtom*> AtomsStorage::selectedAtoms() const {
    QMutexLocker m(&this->_m_handlingLock);
    return this->_selectedAtoms;
}

QVector<snowflake_uid> AtomsStorage::selectedAtomIds() const {
    QMutexLocker l(&this->_m_handlingLock);
    return this->_selectedAtomIds;
}

/////////////
// HISTORY //
/////////////

void AtomsStorage::_registerPayloadForHistory(AlterationPayload &payload) {
    
    //do nothing if payload is from timeline
    if(!payload.isFromTimeline()) return;

    //do nothing if payload is not redo compatible
    if(!payload.isNetworkRoutable()) return;

    //cut branch
    while(this->_payloadHistoryIndex) {
        this->_undoHistory.pop();
        this->_redoHistory.pop();
        this->_payloadHistoryIndex--;
    }

    //store redo
    this->_redoHistory.push(payload);

    //store undo
    auto correspondingUndo = this->_generateUndoPayload(payload);
    this->_undoHistory.push(correspondingUndo);
    
}

void AtomsStorage::undo() {

    //if no history, abort
    auto count = this->_undoHistory.count();
    if(!count) return;

    //check if targeted payload exists
    auto toReach = this->_payloadHistoryIndex + 1;
    auto toReachIndex = count - toReach;
    if(toReachIndex < 0) return;

    //get stored payload and handle it
    auto st_payload = this->_undoHistory.at(toReachIndex);
    st_payload.tagAsFromTimeline();

    //update the index
    this->_payloadHistoryIndex++;

    //propagate
    AlterationHandler::get()->queueAlteration(this, st_payload);

}

void AtomsStorage::redo() {

    //if already on the most recent, abort
    if(!this->_payloadHistoryIndex) return;

    //if no history, abort
    auto count = this->_redoHistory.count();
    if(!count) return;

    //check if targeted payload exists
    auto toReach = this->_payloadHistoryIndex - 1;
    auto toReachIndex = (count - toReach) - 1;
    if(toReachIndex < 0) return;

    //get stored payload and handle it
    auto st_payload = this->_redoHistory.at(toReachIndex);
    st_payload.tagAsFromTimeline();

    //update the index
    this->_payloadHistoryIndex--;

    //process
    AlterationHandler::get()->queueAlteration(this, st_payload);
}

AlterationPayload AtomsStorage::_generateUndoPayload(AlterationPayload &fromHistoryPayload) {

    switch(fromHistoryPayload.type()) {

        case PayloadAlteration::PA_BulkMetadataChanged: {
            
            auto casted = (BulkMetadataChangedPayload*)&fromHistoryPayload;
            auto intialAtoms = casted->atomsUpdates();
            AtomsUpdates out;

            for (auto i = intialAtoms.constBegin(); i != intialAtoms.constEnd(); i++) { 
                
                auto snowflakeId = i.key();
                auto updates = i.value(); 

                auto refAtom = this->_atomsById[snowflakeId];

                AtomUpdates updates;

                for(auto y = updates.begin(); y != updates.end(); y++) {
                    updates.insert(y.key(), refAtom.metadata(y.key()));
                }

                out.insert(snowflakeId, updates);
            }
            
            return BulkMetadataChangedPayload(out);
        }
        break; 

        case PayloadAlteration::PA_MetadataChanged: {
            
            auto casted = (MetadataChangedPayload*)&fromHistoryPayload;
            auto changes = casted->updates();
            AtomsUpdates out;

            for(auto id : casted->targetAtomIds()) {

                AtomUpdates updates;
                auto refAtom = this->_atomsById[id];

                for(auto i = changes.begin(); i != changes.end(); i++) {
                    updates.insert(i.key(), refAtom.metadata(i.key()));
                }

                out.insert(id, updates);
                
            }

            return BulkMetadataChangedPayload(out);
        }
        break; 

        case PayloadAlteration::PA_Added: {
            auto casted = (AddedPayload*)&fromHistoryPayload;
            return RemovedPayload(casted->atoms().keys().toVector());
        }
        break; 

        case PayloadAlteration::PA_Removed: {
            auto casted = (RemovedPayload*)&fromHistoryPayload;
            RPZMap<RPZAtom> out;
            for(auto atomId : casted->targetAtomIds()) {
                out.insert(atomId, this->_atomsById[atomId]);
            }
            return AddedPayload(out);
        }
        break; 

        default:
            break;

    }

    return fromHistoryPayload;

}


/////////////////
// END HISTORY //
/////////////////

//////////////
/* ELEMENTS */
//////////////


RPZAtom* AtomsStorage::_getAtomFromId(const snowflake_uid &id) {
    
    if(!id) {
        qWarning() << "Atoms: targeted Atom Id is null !";
        return nullptr;
    }

    if(!this->_atomsById.contains(id)) return nullptr;
    return &this->_atomsById[id];
}

void AtomsStorage::_bindDefaultOwner(const RPZUser &newOwner) {
    this->_defaultOwner = newOwner;
}

void AtomsStorage::handleAlterationRequest(AlterationPayload &payload) { 
    return this->_handleAlterationRequest(payload);
}

//alter Scene
void AtomsStorage::_handleAlterationRequest(AlterationPayload &payload) { 
    this->_handleAlterationRequest(payload);
}

void AtomsStorage::_handleAlterationRequest(AlterationPayload &payload) {

    QMutexLocker lock(&this->_m_handlingLock);
    
    auto pType = payload.type();
    if(pType == PayloadAlteration::PA_Reset) emit heavyAlterationProcessing();

    //may register for history
    this->_registerPayloadForHistory(payload);

    //on reset
    if(pType == PayloadAlteration::PA_Reset) {
        this->_atomsById.clear();
        this->_atomIdsByOwnerId.clear();
        this->_undoHistory.clear();
        this->_redoHistory.clear();
    }

    //on selection changed
    if(pType == PayloadAlteration::PA_Selected) {
        this->_selectedAtomIds.clear();
        this->_selectedAtoms.clear();
    }

    QHash<snowflake_uid, RPZAtom*> alterations;

    //reset/insert types
    if(auto bPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        for (const auto &atom : bPayload->atoms()) {

            auto storedAtom = this->_insertAtom(atom);
            auto id = atom.id();
            
            alterations.insert(id, storedAtom);
        }

        this->basicAlterationDone(alterations, pType);
    }

    //bulk
    else if(auto bPayload = dynamic_cast<BulkMetadataChangedPayload*>(&payload)) {
        auto updatesById = bPayload->atomsUpdates();
        for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
            
            auto id = i.key();
            auto atom = this->_getAtomFromId(id);

            this->_updateAtom(atom, i.value());
        }

        this->updatesDone(updatesById);
    }

    //multi target format
    else if(auto mPayload = dynamic_cast<MultipleTargetsPayload*>(&payload)) {
        auto ids = mPayload->targetAtomIds();      

        AtomUpdates maybeUpdates;
        RPZUser maybeNewUser;
        
        if(auto nPayload = dynamic_cast<MetadataChangedPayload*>(&payload)) {
            maybeUpdates = nPayload->updates();
        } else if (auto nPayload = dynamic_cast<OwnerChangedPayload*>(&payload)) {
            maybeNewUser = nPayload->newOwner();
        }

        for (const auto &id : ids) {
            auto atom = this->_getAtomFromId(id);
            
            if(pType == PayloadAlteration::PA_Selected) this->_ackSelection(atom);
            if(pType == PayloadAlteration::PA_Removed) this->_removeAtom(atom);
            if(pType == PayloadAlteration::PA_MetadataChanged) this->_updateAtom(atom, maybeUpdates);
            if(pType == PayloadAlteration::PA_OwnerChanged) atom = this->_changeOwner(atom, maybeNewUser);

            if(atom) alterations.insert(id, atom);
        }

        if(!maybeUpdates.isEmpty()) this->updatesDone(alterations.keys(), maybeUpdates);
        else if(!maybeNewUser.isEmpty()) this->ownerChangeDone(alterations.values(), maybeNewUser);
        else this->basicAlterationDone(alterations, pType);

    }
}


//
//
//

RPZAtom* AtomsStorage::_insertAtom(const RPZAtom &newAtom) {
    auto owner = newAtom.owner();
    auto atomId = newAtom.id();

    //bind to owners
    this->_atomIdsByOwnerId[owner.id()].insert(atomId);

    //bind elem
    this->_atomsById.insert(atomId, newAtom);
    
    return &this->_atomsById[atomId];
}

snowflake_uid AtomsStorage::_ackSelection(RPZAtom* selectedAtom) {
    auto id = selectedAtom->id();
    this->_selectedAtomIds.append(id);
    this->_selectedAtoms.append(selectedAtom);
    return selectedAtom->id();
}

RPZAtom* AtomsStorage::_changeOwner(RPZAtom* atomWithNewOwner, const RPZUser &newOwner) {
    auto currentOwnerId = atomWithNewOwner->owner().id();
    auto atomId = atomWithNewOwner->id();

    atomWithNewOwner->setOwnership(newOwner);

    this->_atomIdsByOwnerId[currentOwnerId].remove(atomId);
    this->_atomIdsByOwnerId[newOwner.id()].insert(atomId);

    return atomWithNewOwner;
}

snowflake_uid AtomsStorage::_removeAtom(RPZAtom* toRemove) {
    auto storedAtomOwner = toRemove->owner();
    auto id = toRemove->id();

    //unbind from owners
    this->_atomIdsByOwnerId[storedAtomOwner.id()].remove(id);

    //update 
    this->_atomsById.remove(id); 

    return id;
}

snowflake_uid AtomsStorage::_updateAtom(RPZAtom* toUpdate, const AtomUpdates &updates) {
    toUpdate->setMetadata(updates);
    return toUpdate->id();
}

//
//
//

void AtomsStorage::duplicateAtoms(const QVector<snowflake_uid> &atomIdList) {
    
    //check if a recent duplication have been made, and if it was about the same atoms
    if(this->_latestDuplication != atomIdList) { //if not
        //reset duplication cache
        this->_latestDuplication = atomIdList;
        this->_duplicationCount = 1;
    } else {
        //else, increment subsequent duplication count
        this->_duplicationCount++;
    }
    
    //generate duplicated atoms
    auto newAtoms = this->_generateAtomDuplicates(atomIdList);

    //request insertion
    AddedPayload added(newAtoms);
    AlterationHandler::get()->queueAlteration(this, added);

    //request selection
    SelectedPayload selected(newAtoms.keys().toVector());
    AlterationHandler::get()->queueAlteration(this, selected);
}


RPZMap<RPZAtom> AtomsStorage::_generateAtomDuplicates(const QVector<snowflake_uid> &atomIdsToDuplicate) const {
    
    RPZMap<RPZAtom> newAtoms;

    //create the new atoms from the selection
    for(auto atomId : atomIdsToDuplicate) {
        
        //skip if atomId does not exist
        if(!this->_atomsById.contains(atomId)) continue;
        
        //create copy atom, change ownership to self and update its id
        RPZAtom newAtom(this->_atomsById[atomId]);
        newAtom.shuffleId();
        newAtom.setOwnership(this->_defaultOwner);

        //find new position for the duplicated atom
        auto newPos = _getPositionFromAtomDuplication(newAtom, this->_duplicationCount);
        newAtom.setMetadata(AtomParameter::Position, newPos);

        //adds it to the final list
        newAtoms.insert(newAtom.id(), newAtom);
    }

    return newAtoms;
}

QPointF AtomsStorage::_getPositionFromAtomDuplication(const RPZAtom &atomToDuplicate, int duplicateCount) {
    
    auto currPos = atomToDuplicate.pos();
    auto br = atomToDuplicate.shape().boundingRect();

    auto stepWidth = br.width() / _pixelStepPosDuplication;
    auto stepHeight = br.height() / _pixelStepPosDuplication;
    
    currPos.setX(
        currPos.x() + (duplicateCount * stepWidth)
    );

    currPos.setY(
        currPos.y() + (duplicateCount * stepHeight)
    );

    return currPos;

}


//////////////////
/* END ELEMENTS */
//////////////////