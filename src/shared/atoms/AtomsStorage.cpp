#include "AtomsStorage.h"


AtomsStorage::AtomsStorage(const AlterationPayload::Source &boundSource) : AtomsHandler(boundSource) { };

RPZMap<RPZAtom> AtomsStorage::atoms() {
    return this->_atomsById;
}


//////////////
/* ELEMENTS */
//////////////

//alter Scene
void AtomsStorage::_handlePayload(AlterationPayload* payload) { 
    
    //prevent circular payloads
    auto payloadSource = payload->source();
    if(payloadSource == this->_source) return;

    //on reset
    auto pType = payload->type();

    //on duplication
    if(pType == AlterationPayload::Alteration::Duplicated) {
        return this->_duplicateAtoms(((DuplicatedPayload*)payload)->targetAtomIds());
    }

    if(pType == AlterationPayload::Alteration::Reset) {
        this->_atomsById.clear();
        this->_atomIdsByOwnerId.clear();
    }


    //handling
    auto alterations = payload->alterationByAtomId();
    for (QVariantMap::iterator i = alterations.begin(); i != alterations.end(); ++i) {
        this->_handlePayloadInternal(pType, i.key().toULongLong(), i.value());
    }

    //emit event
    this->_emitAlteration(payload);
}

//register actions
RPZAtom* AtomsStorage::_handlePayloadInternal(const AlterationPayload::Alteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) {

    //get the stored atom relative to the targeted id
    RPZAtom* storedAtom = nullptr;
    if(this->_atomsById.contains(targetedAtomId)) {
        storedAtom = &this->_atomsById[targetedAtomId];
    }

    //modifications
    switch(type) {

        //on addition
        case AlterationPayload::Alteration::Reset:
        case AlterationPayload::Alteration::Added: {
            
            auto newAtom = RPZAtom(atomAlteration.toHash());
            auto owner = newAtom.owner();

            //bind to owners
            this->_atomIdsByOwnerId[owner.id()].insert(targetedAtomId);

            //bind elem
            this->_atomsById.insert(targetedAtomId, newAtom);
            
            //replace for return
            storedAtom = &this->_atomsById[targetedAtomId];
            
        }
        break;
        
        // on locking change
        case AlterationPayload::Alteration::LockChanged: {
            auto isLocked = atomAlteration.toBool();
            auto mdata = storedAtom->metadata();
            mdata.setLocked(isLocked);
            storedAtom->setMetadata(mdata);
        }
        break;
        
        // on changing visibility
        case AlterationPayload::Alteration::VisibilityChanged: {
            auto isHidden = atomAlteration.toBool();
            auto mdata = storedAtom->metadata();
            mdata.setHidden(isHidden);
            storedAtom->setMetadata(mdata);
        }
        break;

        //on move
        case AlterationPayload::Alteration::Moved: {
            auto position = atomAlteration.toPointF();
            auto mdata = storedAtom->metadata();
            mdata.setPos(position);
            storedAtom->setMetadata(mdata);
        }
        break;

        //on scaling
        case AlterationPayload::Alteration::Scaled: {
            auto scale = atomAlteration.toDouble();
            auto mdata = storedAtom->metadata();
            mdata.setScale(scale);
            storedAtom->setMetadata(mdata);
        }
        break;

        //on rotation
        case AlterationPayload::Alteration::Rotated: {
            auto deg = atomAlteration.toDouble();
            auto mdata = storedAtom->metadata();
            mdata.setRotation(deg);
            storedAtom->setMetadata(mdata);
        }
        break;

        //on resize
        case AlterationPayload::Alteration::LayerChanged: {
            auto layer = atomAlteration.toInt();
            auto mdata = storedAtom->metadata();
            mdata.setLayer(layer);
            storedAtom->setMetadata(mdata);
        }
        break;

        //on text change
        case AlterationPayload::Alteration::TextChanged: {
            auto text = atomAlteration.toString();
            auto mdata = storedAtom->metadata();
            mdata.setText(text);
            storedAtom->setMetadata(mdata);
        }
        break;
            
        //on removal
        case AlterationPayload::Alteration::Removed: {
            
            auto storedAtomOwner = storedAtom->owner();

            //unbind from owners
            this->_atomIdsByOwnerId[storedAtomOwner.id()].remove(targetedAtomId);

            //update 
            delete storedAtom->graphicsItem();
            this->_atomsById.remove(targetedAtomId); 
            storedAtom = nullptr;

        }
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
        newAtom.setOwnership(RPZUser());

        auto metadata = newAtom.metadata();
        auto currPos = metadata.pos();
        
        auto br = metadata.shape().boundingRect();
        auto stepWidth = br.width() / 10;
        auto stepHeight = br.height() / 10;
        
            currPos.setX(
                currPos.x() + (this->_duplicationCount * stepWidth)
            );

            currPos.setY(
                currPos.y() + (this->_duplicationCount * stepHeight)
            );

        metadata.setPos(currPos);
        newAtom.setMetadata(metadata);

        newAtoms.insert(newAtom.id(), newAtom);
    }

    //new payload
    this->handleAlterationRequest(AddedPayload(newAtoms));
}


//////////////////
/* END ELEMENTS */
//////////////////