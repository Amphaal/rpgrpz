#include "AtomsStorage.h"

AtomsStorage::AtomsStorage(const AlterationPayload::Source &boundSource) : AlterationAcknoledger(boundSource) { };

RPZMap<RPZAtom> AtomsStorage::atoms() const {
    QMutexLocker m(&this->_m_handlingLock);
    return this->_atomsById;
}

const AtomsSelectionDescriptor AtomsStorage::getAtomSelectionDescriptor(const QVector<RPZAtomId> &selectedIds) const {
    
    AtomsSelectionDescriptor out;
    out.selectedAtomIds = selectedIds;

    {
        QMutexLocker m(&this->_m_handlingLock);
        
        if(selectedIds.count() == 1) {
            out.templateAtom = this->_atomsById[selectedIds[0]];
            out.representedTypes += out.templateAtom.type();
        }

        else {
            for(auto id : selectedIds) {
                out.representedTypes += this->_atomsById[id].type();
            }
        }

    }

    return out;
}

QVector<RPZAtomId> AtomsStorage::bufferedSelectedAtomIds() const {
    QMutexLocker l(&this->_m_handlingLock);
    return this->_selectedRPZAtomIds;
}

/////////////
// HISTORY //
/////////////

void AtomsStorage::_registerPayloadForHistory(AlterationPayload &payload) {
    
    //do not register again if payload is already from timeline
    if(payload.isFromTimeline()) return;

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

                AtomUpdates oldValues;

                for(auto y = updates.begin(); y != updates.end(); y++) {
                    auto param = y.key();
                    auto oldValue = refAtom.metadata(param);
                    oldValues.insert(param, oldValue);
                }

                out.insert(snowflakeId, oldValues);
            }
            
            return BulkMetadataChangedPayload(out);
        }
        break; 

        case PayloadAlteration::PA_MetadataChanged: {
            
            auto casted = (MetadataChangedPayload*)&fromHistoryPayload;
            auto changes = casted->updates();
            AtomsUpdates out;

            for(auto id : casted->targetRPZAtomIds()) {

                AtomUpdates oldValues;
                auto refAtom = this->_atomsById[id];

                for(auto i = changes.begin(); i != changes.end(); i++) {
                    auto param = i.key();
                    auto oldValue = refAtom.metadata(param);
                    oldValues.insert(param, oldValue);
                }

                out.insert(id, oldValues);
                
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
            for(auto RPZAtomId : casted->targetRPZAtomIds()) {
                out.insert(RPZAtomId, this->_atomsById[RPZAtomId]);
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


RPZAtom* AtomsStorage::_getAtomFromId(const RPZAtomId &id) {
    
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

void AtomsStorage::_handleAlterationRequest(AlterationPayload &payload) {

    QMutexLocker lock(&this->_m_handlingLock);
    
    auto pType = payload.type();
    if(pType == PayloadAlteration::PA_Reset) emit heavyAlterationProcessing();
    
    //may register for history
    this->_registerPayloadForHistory(payload);

    //on reset
    if(pType == PayloadAlteration::PA_Reset) {
        this->_atomsById.clear();
        this->_RPZAtomIdsByOwnerId.clear();
        this->_undoHistory.clear();
        this->_redoHistory.clear();
    }

    //on selection changed
    if(pType == PayloadAlteration::PA_Selected) {
        this->_selectedRPZAtomIds.clear();
        this->_selectedAtoms.clear();
    }

    QList<RPZAtomId> alterationsIds;

    //reset/insert types
    if(auto mPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        for (const auto &atom : mPayload->atoms()) {

            auto storedAtom = this->_insertAtom(atom);
            auto id = atom.id();
            
            alterationsIds += id;
        }

        this->_basicAlterationDone(alterationsIds, pType);
    }

    //bulk
    else if(auto mPayload = dynamic_cast<BulkMetadataChangedPayload*>(&payload)) {
        auto updatesById = mPayload->atomsUpdates();
        for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
            
            auto id = i.key();
            auto atom = this->_getAtomFromId(id);

            this->_updateAtom(atom, i.value());
        }

        this->_updatesDone(updatesById);
    }

    //multi target format
    else if(auto mPayload = dynamic_cast<MultipleTargetsPayload*>(&payload)) {
        auto ids = mPayload->targetRPZAtomIds();      

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

            if(atom) alterationsIds += id;
        }

        if(!maybeUpdates.isEmpty()) this->_updatesDone(alterationsIds, maybeUpdates);
        else if(!maybeNewUser.isEmpty()) this->_ownerChangeDone(alterationsIds, maybeNewUser);
        else this->_basicAlterationDone(alterationsIds, pType);

    }
}


//
//
//

RPZAtom* AtomsStorage::_insertAtom(const RPZAtom &newAtom) {
    auto owner = newAtom.owner();
    auto RPZAtomId = newAtom.id();

    //bind to owners
    this->_RPZAtomIdsByOwnerId[owner.id()].insert(RPZAtomId);

    //bind elem
    this->_atomsById.insert(RPZAtomId, newAtom);
    
    return &this->_atomsById[RPZAtomId];
}

RPZAtomId AtomsStorage::_ackSelection(RPZAtom* selectedAtom) {
    auto id = selectedAtom->id();
    this->_selectedRPZAtomIds.append(id);
    this->_selectedAtoms.append(selectedAtom);
    return selectedAtom->id();
}

RPZAtom* AtomsStorage::_changeOwner(RPZAtom* atomWithNewOwner, const RPZUser &newOwner) {
    auto currentOwnerId = atomWithNewOwner->owner().id();
    auto RPZAtomId = atomWithNewOwner->id();

    atomWithNewOwner->setOwnership(newOwner);

    this->_RPZAtomIdsByOwnerId[currentOwnerId].remove(RPZAtomId);
    this->_RPZAtomIdsByOwnerId[newOwner.id()].insert(RPZAtomId);

    return atomWithNewOwner;
}

RPZAtomId AtomsStorage::_removeAtom(RPZAtom* toRemove) {
    auto storedAtomOwner = toRemove->owner();
    auto id = toRemove->id();

    //unbind from owners
    this->_RPZAtomIdsByOwnerId[storedAtomOwner.id()].remove(id);

    //update 
    this->_atomsById.remove(id); 

    return id;
}

RPZAtomId AtomsStorage::_updateAtom(RPZAtom* toUpdate, const AtomUpdates &updates) {
    toUpdate->setMetadata(updates);
    return toUpdate->id();
}

//
//
//

void AtomsStorage::duplicateAtoms(const QVector<RPZAtomId> &RPZAtomIdList) {
    
    //check if a recent duplication have been made, and if it was about the same atoms
    if(this->_latestDuplication != RPZAtomIdList) { //if not
        //reset duplication cache
        this->_latestDuplication = RPZAtomIdList;
        this->_duplicationCount = 1;
    } else {
        //else, increment subsequent duplication count
        this->_duplicationCount++;
    }
    
    //generate duplicated atoms
    auto newAtoms = this->_generateAtomDuplicates(RPZAtomIdList);

    //request insertion
    AddedPayload added(newAtoms);
    AlterationHandler::get()->queueAlteration(this, added);

    //request selection
    SelectedPayload selected(newAtoms.keys().toVector());
    AlterationHandler::get()->queueAlteration(this, selected);
}


RPZMap<RPZAtom> AtomsStorage::_generateAtomDuplicates(const QVector<RPZAtomId> &RPZAtomIdsToDuplicate) const {
    
    RPZMap<RPZAtom> newAtoms;

    //create the new atoms from the selection
    for(auto RPZAtomId : RPZAtomIdsToDuplicate) {
        
        //skip if RPZAtomId does not exist
        if(!this->_atomsById.contains(RPZAtomId)) continue;
        
        //create copy atom, change ownership to self and update its id
        RPZAtom newAtom(this->_atomsById[RPZAtomId]);
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