#include "AtomsStorage.h"

AtomsStorage::AtomsStorage(const Payload::Source &boundSource) : AlterationAcknoledger(boundSource) { };

const ResetPayload AtomsStorage::generateResetPayload() const {
    QMutexLocker l(&_m_handlingLock);
    return ResetPayload(this->_map);
}

MapDatabase& AtomsStorage::map() {
    return this->_map;
}

const MapDatabase& AtomsStorage::map() const {
    return this->_map;
}

void AtomsStorage::_replaceMap(const MapDatabase &map) {
    QMutexLocker l(&_m_handlingLock);
    this->_map = map;
}

PossibleActionsOnAtomList AtomsStorage::getPossibleActions(const QList<RPZAtom::Id> &ids) {
    
    QMutexLocker l(&_m_handlingLock);
    PossibleActionsOnAtomList out;

    //no actions possible if not host able
    if(RPZClient::isHostAble()) return out;

    //availability
    auto areIdsSelected = !ids.isEmpty();
    out.canChangeAvailability = areIdsSelected;

    // redo/undo
    out.somethingRedoable = this->_canRedo() > -1;
    out.somethingUndoable = this->_canUndo() > -1;

    auto containsAnInteractiveAtom = false;

    //iterate
    QList<const RPZAtom*> atomList;
    for(auto &id : ids) {

        //get atom
        auto atom = this->_map.atomPtr(id);

        if(atom->category() == RPZAtom::Category::Interactive) 
            containsAnInteractiveAtom = true;

        if(!atom) continue;

        atomList += atom;

        //if is locked, break
        if(atom->isLocked()) {
            return out;
        }

    }

    //else, activate most
    out.canChangeLayer = areIdsSelected && !containsAnInteractiveAtom;
    out.canCopy = areIdsSelected;
    out.canChangeVisibility = areIdsSelected && !containsAnInteractiveAtom;
    out.canRemove = areIdsSelected;

    //determine min/max
    if(out.canChangeLayer) {
        auto minMaxLayer = this->_determineMinMaxLayer(atomList);
        out.targetDownLayer = minMaxLayer.first;
        out.targetUpLayer = minMaxLayer.second;
    }

    return out;
}

QPair<int, int> AtomsStorage::_determineMinMaxLayer(const QList<const RPZAtom*> &atoms) {
    
    //targets
    auto firstPass = true;
    auto riseLayoutTarget = 0;
    auto lowerLayoutTarget = 0;

    for(auto atom : atoms) {
        
        auto layer = atom->layer();
        
        if(firstPass) {
            firstPass = false;
            riseLayoutTarget = layer;
            lowerLayoutTarget = layer;
            continue;
        }

        if(layer > riseLayoutTarget) riseLayoutTarget = layer;
        if(layer < lowerLayoutTarget) lowerLayoutTarget = layer;
    }

    riseLayoutTarget++;
    lowerLayoutTarget--;

    return QPair<int, int>(lowerLayoutTarget, riseLayoutTarget);
}

const AtomsSelectionDescriptor AtomsStorage::getAtomSelectionDescriptor(const QList<RPZAtom::Id> &selectedIds) const {
    
    AtomsSelectionDescriptor out;

    out.selectedAtomIds = selectedIds;
    auto takeFirstAtomAsTemplate = selectedIds.count() == 1;

    {
        QMutexLocker m(&this->_m_handlingLock);
        
        for(auto id : selectedIds) {

            auto atom = this->_map.atom(id);
            if(atom.isEmpty()) continue;

            out.representedTypes.insert(atom.type());

            if(takeFirstAtomAsTemplate) out.templateAtom = atom;
            
        }

    }

    return out;
}

AtomsStorage::AtomsAreLeft AtomsStorage::restrictPayload(AtomRelatedPayload &payloadToRestrict) {
    
    //no need to touch
    if(!this->_restrictedAtomIds.count()) return true;

    //restrict
    return payloadToRestrict.restrictTargetedAtoms(this->_restrictedAtomIds);

}

/////////////
// HISTORY //
/////////////

void AtomsStorage::_registerPayloadForHistory(const AlterationPayload &payload) {
    
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

int AtomsStorage::_canRedo() {

    //if already on the most recent, abort
    if(!this->_payloadHistoryIndex) return -1;

    //if no history, abort
    auto count = this->_redoHistory.count();
    if(!count) return -1;

    //check if targeted payload exists
    auto toReach = this->_payloadHistoryIndex - 1;
    auto toReachIndex = (count - toReach) - 1;

    return toReachIndex;

}

int AtomsStorage::_canUndo() {
    
    //if no history, abort
    auto count = this->_undoHistory.count();
    if(!count) return -1;

    //check if targeted payload exists
    auto toReach = this->_payloadHistoryIndex + 1;
    auto toReachIndex = count - toReach;

    return toReachIndex;

}

void AtomsStorage::undo() {

    auto toReachIndex = this->_canUndo();
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

     auto toReachIndex = this->_canRedo();
    if(toReachIndex < 0) return;

    //get stored payload and handle it
    auto st_payload = this->_redoHistory.at(toReachIndex);
    st_payload.tagAsFromTimeline();

    //update the index
    this->_payloadHistoryIndex--;

    //process
    AlterationHandler::get()->queueAlteration(this, st_payload);
}

AlterationPayload AtomsStorage::_generateUndoPayload(const AlterationPayload &fromHistoryPayload) {

    switch(fromHistoryPayload.type()) {

        case Payload::Alteration::BulkMetadataChanged: {
            
            auto casted = (BulkMetadataChangedPayload*)&fromHistoryPayload;
            auto intialAtoms = casted->atomsUpdates();
            
            RPZAtom::ManyUpdates out;
            for (auto i = intialAtoms.constBegin(); i != intialAtoms.constEnd(); i++) { 
                
                //get atom
                auto snowflakeId = i.key();
                auto atom = this->_map.atomPtr(snowflakeId);
                if(!atom) continue;

                //init hash with old values
                RPZAtom::Updates oldValues;
                for(auto &param : i.value().keys()) {
                    auto oldValue = atom->metadata(param);
                    oldValues.insert(param, oldValue);
                }

                out.insert(snowflakeId, oldValues);
            }
            
            return BulkMetadataChangedPayload(out);

        }
        break; 

        case Payload::Alteration::MetadataChanged: {
            
            auto casted = (MetadataChangedPayload*)&fromHistoryPayload;
            
            RPZAtom::ManyUpdates out;
            for(auto &id : casted->targetRPZAtomIds()) {

                //get atom
                auto atom = this->_map.atomPtr(id);
                if(!atom) continue;

                RPZAtom::Updates oldValues;
                for(auto &param : casted->updates().keys()) {
                    auto oldValue = atom->metadata(param);
                    oldValues.insert(param, oldValue);
                }

                out.insert(id, oldValues);
                
            }

            return BulkMetadataChangedPayload(out);

        }
        break; 

        case Payload::Alteration::Added: {
            auto casted = (AddedPayload*)&fromHistoryPayload;
            return RemovedPayload(casted->atoms().keys());
        }
        break; 

        case Payload::Alteration::Removed: {
            
            auto casted = (RemovedPayload*)&fromHistoryPayload;
            
            QList<RPZAtom> out;
            for(auto &id : casted->targetRPZAtomIds()) {
                
                auto atom = this->_map.atom(id);
                if(atom.isEmpty()) continue;

                out += atom;

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

void AtomsStorage::handleAlterationRequest(const AlterationPayload &payload) { 
    return this->_handleAlterationRequest(payload);
}

void AtomsStorage::_handleAlterationRequest(const AlterationPayload &payload) {

    QMutexLocker lock(&this->_m_handlingLock);
    
    auto pType = payload.type();
    
    //may register for history
    this->_registerPayloadForHistory(payload);

    //on reset
    if(pType == Payload::Alteration::Reset) {
        this->_map.clear();
        this->_undoHistory.clear();
        this->_redoHistory.clear();
    }

    //reset/insert types
    if(auto mPayload = dynamic_cast<const AtomsWielderPayload*>(&payload)) {
        
        QList<RPZAtom::Id> insertedIds;
        
        for (const auto &atom : mPayload->atoms()) {

            //add to Db
            auto id = atom.id();
            this->_map.addAtom(atom);
            
            //add to restricted
            if(atom.isRestrictedAtom()) {
                this->_restrictedAtomIds += id;
            }

            //handler for inheritors
            this->_atomAdded(atom);

            insertedIds += id;

        }

        this->_basicAlterationDone(insertedIds, pType);
        
    }

    //bulk
    else if(auto mPayload = dynamic_cast<const BulkMetadataChangedPayload*>(&payload)) {
        auto updatesById = mPayload->atomsUpdates();
        for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
            
            auto id = i.key();
            auto atom = this->_map.atom(i.key());
            if(atom.isEmpty()) continue;

            this->_map.updateAtom(id, i.value());
            
        }

        this->_updatesDone(updatesById);
    }

    //multi target format
    else if(auto mPayload = dynamic_cast<const MultipleAtomTargetsPayload*>(&payload)) {
        
        QList<RPZAtom::Id> alteredIds;
        RPZAtom::Updates maybeUpdates;
        
        if(auto nPayload = dynamic_cast<const MetadataChangedPayload*>(&payload)) {
            maybeUpdates = nPayload->updates();
        } 

        for (const auto &id : mPayload->targetRPZAtomIds()) {

            auto atom = this->_map.atom(id);
            if(atom.isEmpty()) continue;
            
            if(pType == Payload::Alteration::Removed) {
                this->_map.removeAtom(id); //remove from db
                this->_restrictedAtomIds.remove(id); //remove from restricted
            }
            if(pType == Payload::Alteration::MetadataChanged) this->_map.updateAtom(id, maybeUpdates);

            alteredIds += id;

        }

        if(!maybeUpdates.isEmpty()) this->_updatesDone(alteredIds, maybeUpdates);
        else this->_basicAlterationDone(alteredIds, pType);

    }
}


//
//
//

void AtomsStorage::duplicateAtoms(const QList<RPZAtom::Id> &RPZAtomIdList) {
    
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
    if(!newAtoms.count()) return;

    //request insertion
    AddedPayload added(newAtoms.values());
    AlterationHandler::get()->queueAlteration(this, added);

    //request selection
    SelectedPayload selected(newAtoms.keys());
    AlterationHandler::get()->queueAlteration(this, selected);
}


RPZMap<RPZAtom> AtomsStorage::_generateAtomDuplicates(const QList<RPZAtom::Id> &RPZAtomIdsToDuplicate) const {
    
    RPZMap<RPZAtom> newAtoms;

    //create the new atoms from the selection
    for(auto &atomId : RPZAtomIdsToDuplicate) {
        
        //skip if RPZAtom::Id does not exist
        auto atom = this->_map.atom(atomId);
        if(atom.isEmpty()) continue;
        
        //create copy atom, update its id
        RPZAtom newAtom(atom);
        newAtom.shuffleId();

        //find new position for the duplicated atom
        auto newPos = _getPositionFromAtomDuplication(newAtom, this->_duplicationCount);
        newAtom.setMetadata(RPZAtom::Parameter::Position, newPos);

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