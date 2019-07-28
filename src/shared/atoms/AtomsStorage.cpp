#include "AtomsStorage.h"


AtomsStorage::AtomsStorage(const AlterationPayload::Source &boundSource) : AtomsHandler(boundSource) { };

RPZMap<RPZAtom> AtomsStorage::atoms() {
    return this->_atomsById;
}

/////////////
// HISTORY //
/////////////

void AtomsStorage::_registerPayloadForHistory(AlterationPayload &payload) {
    
    //do nothing is payload is not redo compatible
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
    auto ptr = Payloads::autoCast(st_payload);

    //change source
    ptr->changeSource(AlterationPayload::Source::Undefined);

    this->_basic_handlePayload(*ptr);

    //update the index
    this->_payloadHistoryIndex++;
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
    auto ptr = Payloads::autoCast(st_payload);

    //change source
    ptr->changeSource(AlterationPayload::Source::Undefined);

    this->_basic_handlePayload(*ptr);

    //update the index
    this->_payloadHistoryIndex--;
}

AlterationPayload AtomsStorage::_generateUndoPayload(AlterationPayload &historyPayload) {

    switch(historyPayload.type()) {

        case PayloadAlteration::BulkMetadataChanged: {
            
            auto casted = (BulkMetadataChangedPayload*)&historyPayload;
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

        case PayloadAlteration::MetadataChanged: {
            
            auto casted = (MetadataChangedPayload*)&historyPayload;
            auto changesTypes = MetadataChangedPayload::fromArgs(casted->args()).editedMetadata();
            RPZMap<RPZAtom> partialAtoms;

            for(auto id : casted->targetAtomIds()) {

                RPZAtom baseAtom;
                auto refAtom = this->_atomsById[id];

                for(auto change : changesTypes) {
                    baseAtom.setMetadata(change, change);
                }

                partialAtoms.insert(id, baseAtom);
                
            }

            return BulkMetadataChangedPayload(partialAtoms);
        }
        break; 

        case PayloadAlteration::Added: {
            auto casted = (AddedPayload*)&historyPayload;
            return RemovedPayload(casted->atoms().keys().toVector());
        }
        break; 

        case PayloadAlteration::Removed: {
            auto casted = (RemovedPayload*)&historyPayload;
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

    return historyPayload;
}


/////////////////
// END HISTORY //
/////////////////

//////////////
/* ELEMENTS */
//////////////

//alter Scene
void AtomsStorage::_handlePayload(AlterationPayload &payload) { 

    //prevent circular payloads
    if(payload.source() == this->_source) return;

    auto pType = payload.type();
    if(pType == PayloadAlteration::Redone) return this->redo(); //on redo
    if(pType == PayloadAlteration::Undone) return this->undo(); //on undo

    //on duplication
    if(auto dCasted = dynamic_cast<DuplicatedPayload*>(&payload)) {
        auto targetAtomsIds = dCasted->targetAtomIds();
        return this->_duplicateAtoms(targetAtomsIds);
    }

    //register history
    this->_registerPayloadForHistory(payload);

    //on reset
    if(pType == PayloadAlteration::Reset) {
        this->_atomsById.clear();
        this->_atomIdsByOwnerId.clear();
        this->_undoHistory.clear();
        this->_redoHistory.clear();
    }

    //base handling
    this->_basic_handlePayload(payload);
}

void AtomsStorage::_basic_handlePayload(AlterationPayload &payload) {

    auto type = payload.type();

    //atom wielders format
    if(auto bPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        
        auto atoms  = bPayload->atoms();

        for (RPZMap<RPZAtom>::iterator i = atoms.begin(); i != atoms.end(); ++i) {
            this->_handlePayloadInternal(type, i.key(), i.value());
        }

    }

    //multi target format
    if(auto mPayload = dynamic_cast<MultipleTargetsPayload*>(&payload)) {
        
        auto ids = mPayload->targetAtomIds();
        auto args =  mPayload->args();
        
        for (auto id : ids) {
            this->_handlePayloadInternal(type, id, args);
        }

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
        case PayloadAlteration::Reset:
        case PayloadAlteration::Added: {
            
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

        case PayloadAlteration::MetadataChanged:
        case PayloadAlteration::BulkMetadataChanged: {
            auto partial = type == PayloadAlteration::BulkMetadataChanged ? 
                                                    RPZAtom(alteration.toHash()) : 
                                                    MetadataChangedPayload::fromArgs(alteration);
            
            for(auto param : partial.editedMetadata()) {
                storedAtom->setMetadata(param, partial);
            }
        }   
        break;

        //on removal
        case PayloadAlteration::Removed: {
            
            auto storedAtomOwner = storedAtom->owner();

            //unbind from owners
            this->_atomIdsByOwnerId[storedAtomOwner.id()].remove(targetedAtomId);

            //update 
            delete storedAtom->graphicsItem();
            this->_atomsById.remove(targetedAtomId); 
            storedAtom = nullptr;

        }
        break;

        default:
            break;

    }

    return storedAtom;
}

void AtomsStorage::_duplicateAtoms(QVector<snowflake_uid> &atomIdList) {
    
    if(this->_latestDuplication != atomIdList) {
        this->_latestDuplication = atomIdList;
        this->_duplicationCount = 1;
    } else {
        this->_duplicationCount++;
    }
    
    RPZMap<RPZAtom> newAtoms;
    for(auto atomId : atomIdList) {
        
        if(!this->_atomsById.contains(atomId)) continue;

        RPZAtom newAtom(this->_atomsById[atomId]);
        newAtom.shuffleId();
        newAtom.setOwnership(this->_defaultOwner);

        auto currPos = newAtom.pos();
        
        auto br = newAtom.shape().boundingRect();
        auto stepWidth = br.width() / 10;
        auto stepHeight = br.height() / 10;
        
            currPos.setX(
                currPos.x() + (this->_duplicationCount * stepWidth)
            );

            currPos.setY(
                currPos.y() + (this->_duplicationCount * stepHeight)
            );

        newAtom.setMetadata(AtomParameter::Position, currPos);

        newAtoms.insert(newAtom.id(), newAtom);
    }

    //add them
    AddedPayload added(newAtoms);
    this->handleAlterationRequest(added);

    //select them 
    SelectedPayload selected(newAtoms.keys().toVector());
    this->handleAlterationRequest(selected);
}


//////////////////
/* END ELEMENTS */
//////////////////