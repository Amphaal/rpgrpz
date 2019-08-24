#include "AtomsStorage.h"


AtomsStorage::AtomsStorage(const AlterationPayload::Source &boundSource, bool autoLinkage) : AlterationAcknoledger(boundSource, autoLinkage) { };

RPZMap<RPZAtom> AtomsStorage::atoms() const {
    return this->_atomsById;
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
            auto intialAtoms = casted->atoms();
            RPZMap<RPZAtom> outAtoms;

            for (RPZMap<RPZAtom>::iterator i = intialAtoms.begin(); i != intialAtoms.end(); ++i) { 
                
                auto snowflakeId = i.key();
                auto partialAtom = i.value(); 

                RPZAtom outAtom;
                auto refAtom = this->_atomsById[snowflakeId];

                for(auto change : partialAtom.editedMetadata()) {
                    outAtom.setMetadata(change, refAtom);
                }

                outAtoms.insert(snowflakeId, outAtom);
            }
            
            return BulkMetadataChangedPayload(outAtoms);
        }
        break; 

        case PayloadAlteration::PA_MetadataChanged: {
            
            auto casted = (MetadataChangedPayload*)&fromHistoryPayload;
            auto changes = casted->updates();
            RPZMap<RPZAtom> partialAtoms;

            for(auto id : casted->targetAtomIds()) {

                RPZAtom baseAtom;
                auto refAtom = this->_atomsById[id];

                for(auto i = changes.begin(); i != changes.end(); i++) {
                    baseAtom.setMetadata(i.key(), i.value());
                }

                partialAtoms.insert(id, baseAtom);
                
            }

            return BulkMetadataChangedPayload(partialAtoms);
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

void AtomsStorage::handleAlterationRequest(AlterationPayload &payload) { 
    return this->_handleAlterationRequest(payload);
}

//alter Scene
void AtomsStorage::_handleAlterationRequest(AlterationPayload &payload) { 

    //may register for history
    this->_registerPayloadForHistory(payload);

    auto pType = payload.type();

    //on reset
    if(pType == PayloadAlteration::PA_Reset) {
        this->_atomsById.clear();
        this->_atomIdsByOwnerId.clear();
        this->_undoHistory.clear();
        this->_redoHistory.clear();
    }

    //on selection changed
    if(pType == PayloadAlteration::PA_Selected) {
        this->_m_selectedAtomIds.lock();
        this->_selectedAtomIds.clear();
    }

    //base handling
    if(auto bPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        
        auto atoms  = bPayload->atoms();

        for (auto i = atoms.begin(); i != atoms.end(); ++i) {
            this->_handlePayloadInternal(pType, i.key(), i.value());
        }

    }

    //multi target format
    if(auto mPayload = dynamic_cast<MultipleTargetsPayload*>(&payload)) {
        
        auto ids = mPayload->targetAtomIds();
        auto args = mPayload->args();
        
        for (auto id : ids) {
            this->_handlePayloadInternal(pType, id, args);
        }

    }

    //unlock 
    if(pType == PayloadAlteration::PA_Selected) {
        this->_m_selectedAtomIds.unlock();
    }

}


//register actions
RPZAtom* AtomsStorage::_handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) {

    if(!targetedAtomId) {
        qWarning() << "Atoms: targeted Atom Id is null !";
    }

    //get the stored atom relative to the targeted id
    RPZAtom* storedAtom = nullptr;
    if(this->_atomsById.contains(targetedAtomId)) {
        storedAtom = &this->_atomsById[targetedAtomId];
    }

    //modifications
    switch(type) {

        //on addition
        case PayloadAlteration::PA_Reset:
        case PayloadAlteration::PA_Added: {
            
            auto newAtom = RPZAtom(alteration.toHash());
            auto owner = newAtom.owner();

            //bind to owners
            this->_atomIdsByOwnerId[owner.id()].insert(targetedAtomId);

            //bind elem
            this->_atomsById.insert(targetedAtomId, newAtom);
            
            //replace for return
            storedAtom = &this->_atomsById[targetedAtomId];
            
        }
        break;

        //on owner change
        case PayloadAlteration::PA_OwnerChanged: {

            auto currentOwnerId = storedAtom->owner().id();

            RPZUser newOwner(alteration.toHash());
            storedAtom->setOwnership(newOwner);

            this->_atomIdsByOwnerId[currentOwnerId].remove(targetedAtomId);
            this->_atomIdsByOwnerId[newOwner.id()].insert(targetedAtomId);
        }
        break;

        //on selection change
        case PayloadAlteration::PA_Selected: {
            this->_selectedAtomIds.append(targetedAtomId);
        }
        break;

        case PayloadAlteration::PA_MetadataChanged:
        case PayloadAlteration::PA_BulkMetadataChanged: {
            
            auto changes = type == PayloadAlteration::PA_BulkMetadataChanged ? 
                                    RPZAtom(alteration.toHash()).editedMetadataWithValues() : 
                                    MetadataChangedPayload::fromArgs(alteration);
            
            for(auto i = changes.begin(); i != changes.end(); i++) {
                storedAtom->setMetadata(i.key(), i.value());
            }

        }   
        break;

        //on removal
        case PayloadAlteration::PA_Removed: {
            
            auto storedAtomOwner = storedAtom->owner();

            //unbind from owners
            this->_atomIdsByOwnerId[storedAtomOwner.id()].remove(targetedAtomId);

            //update 
            this->_atomsById.remove(targetedAtomId); 
            storedAtom = nullptr;

        }
        break;

        default:
            break;

    }

    return storedAtom;
}

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

QVector<snowflake_uid> AtomsStorage::selectedAtomIds() const {
    QMutexLocker l(&this->_m_selectedAtomIds);
    return this->_selectedAtomIds;
}

//////////////////
/* END ELEMENTS */
//////////////////