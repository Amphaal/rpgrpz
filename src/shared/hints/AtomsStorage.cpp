#include "AtomsStorage.h"

AtomsStorage::AtomsStorage(const Payload::Source &boundSource) : AlterationAcknoledger(boundSource) { };

const ResetPayload AtomsStorage::generateResetPayload() const {
    QMutexLocker l(&_m_handlingLock);
    return ResetPayload(this->_map);
}

PossibleActionsOnAtomList AtomsStorage::getPossibleActions(const QVector<RPZAtomId> &ids) {
    
    QMutexLocker l(&_m_handlingLock);
    PossibleActionsOnAtomList out;

    //availability
    auto areIdsSelected = !ids.isEmpty();
    out.canChangeAvailability = areIdsSelected;

    // redo/undo
    out.somethingRedoable = this->_canRedo() > -1;
    out.somethingUndoable = this->_canUndo() > -1;

    //iterate
    QList<const RPZAtom*> atomList;
    for(auto &id : ids) {

        //get atom
        auto atom = this->_map.atom(id);
        if(!atom) continue;

        atomList += atom;

        //if is locked, break
        if(atom->isLocked()) {
            return out;
        }

    }

    //else, activate most
    out.canChangeLayer = areIdsSelected;
    out.canCopy = areIdsSelected;
    out.canChangeVisibility = areIdsSelected;
    out.canRemove = areIdsSelected;

    //determine min/max
    auto minMaxLayer = this->_determineMinMaxLayer(atomList);
    out.targetDownLayer = minMaxLayer.first;
    out.targetUpLayer = minMaxLayer.second;

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

const AtomsSelectionDescriptor AtomsStorage::getAtomSelectionDescriptor(const QVector<RPZAtomId> &selectedIds) const {
    
    AtomsSelectionDescriptor out;

    out.selectedAtomIds = selectedIds;
    auto takeFirstAtomAsTemplate = selectedIds.count() == 1;

    {
        QMutexLocker m(&this->_m_handlingLock);
        
        for(auto id : selectedIds) {

            auto atom = this->_map.atomAsCopy(id);
            if(atom.isEmpty()) continue;

            out.representedTypes.insert(atom.type());

            if(takeFirstAtomAsTemplate) out.templateAtom = atom;
            
        }

    }

    return out;
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

AlterationPayload AtomsStorage::_generateUndoPayload(AlterationPayload &fromHistoryPayload) {

    switch(fromHistoryPayload.type()) {

        case Payload::Alteration::BulkMetadataChanged: {
            
            auto casted = (BulkMetadataChangedPayload*)&fromHistoryPayload;
            auto intialAtoms = casted->atomsUpdates();
            
            AtomsUpdates out;
            for (auto i = intialAtoms.constBegin(); i != intialAtoms.constEnd(); i++) { 
                
                //get atom
                auto snowflakeId = i.key();
                auto atom = this->_map.atom(snowflakeId);
                if(!atom) continue;

                //init hash with old values
                AtomUpdates oldValues;
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
            
            AtomsUpdates out;
            for(auto &id : casted->targetRPZAtomIds()) {

                //get atom
                auto atom = this->_map.atom(id);
                if(!atom) continue;

                AtomUpdates oldValues;
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
            return RemovedPayload(casted->atoms().keys().toVector());
        }
        break; 

        case Payload::Alteration::Removed: {
            
            auto casted = (RemovedPayload*)&fromHistoryPayload;
            
            QList<RPZAtom> out;
            for(auto &id : casted->targetRPZAtomIds()) {
                
                auto atom = this->_map.atom(id);
                if(!atom) continue;

                out += *atom;

            }

            return AddedPayload::fromAtoms(out);

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

void AtomsStorage::_handleAlterationRequest(AlterationPayload &payload) {

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
    if(auto mPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        
        QList<RPZAtomId> insertedIds;
        
        for (const auto &atom : mPayload->atoms()) {

            this->_map.addAtom(atom);
            this->_atomAdded(atom);

            auto id = atom.id();
            insertedIds += id;

        }

        this->_basicAlterationDone(insertedIds, pType);
        
    }

    //bulk
    else if(auto mPayload = dynamic_cast<BulkMetadataChangedPayload*>(&payload)) {
        auto updatesById = mPayload->atomsUpdates();
        for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
            
            auto id = i.key();
            auto atom = this->_map.atom(i.key());
            if(!atom) continue;

            this->_map.updateAtom(id, i.value());
            
        }

        this->_updatesDone(updatesById);
    }

    //multi target format
    else if(auto mPayload = dynamic_cast<MultipleAtomTargetsPayload*>(&payload)) {
        
        QList<RPZAtomId> alteredIds;
        AtomUpdates maybeUpdates;
        
        if(auto nPayload = dynamic_cast<MetadataChangedPayload*>(&payload)) {
            maybeUpdates = nPayload->updates();
        } 

        for (const auto &id : mPayload->targetRPZAtomIds()) {

            auto atom = this->_map.atom(id);
            if(!atom) continue;
            
            if(pType == Payload::Alteration::Removed) this->_map.removeAtom(id);
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
    if(!newAtoms.count()) return;

    //request insertion
    auto added = AddedPayload::fromAtoms(newAtoms.values());
    AlterationHandler::get()->queueAlteration(this, added);

    //request selection
    SelectedPayload selected(newAtoms.keys().toVector());
    AlterationHandler::get()->queueAlteration(this, selected);
}


RPZMap<RPZAtom> AtomsStorage::_generateAtomDuplicates(const QVector<RPZAtomId> &RPZAtomIdsToDuplicate) const {
    
    RPZMap<RPZAtom> newAtoms;

    //create the new atoms from the selection
    for(auto &atomId : RPZAtomIdsToDuplicate) {
        
        //skip if RPZAtomId does not exist
        auto atom = this->_map.atomAsCopy(atomId);
        if(atom.isEmpty()) continue;
        
        //create copy atom, update its id
        RPZAtom newAtom(atom);
        newAtom.shuffleId();

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