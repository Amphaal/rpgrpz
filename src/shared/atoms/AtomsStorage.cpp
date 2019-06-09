#include "AtomsStorage.h"


AtomsStorage::AtomsStorage(const AlterationPayload::Source &boundSource) : AtomsHandler(boundSource) { };

RPZMap<RPZAtom> AtomsStorage::atoms() {
    return this->_atomsById;
}


//////////////
/* ELEMENTS */
//////////////

void AtomsStorage::_registerUndoPayload(AlterationPayload* payload) {
    switch(payload->type()) {

            case PayloadAlteration::Moved: {
                auto casted = (MovedPayload*)payload;
            }
            break; 

            case PayloadAlteration::Added: {
                auto casted = (AddedPayload*)payload;
                return RemovedPayload(casted->atoms().keys().toVector());
            }
            break; 

            case PayloadAlteration::Removed: {
                auto casted = (RemovedPayload*)payload;
                RPZMap<RPZAtom> out;
                for(auto atomId : casted->targetAtomIds()) {
                    out.insert(atomId, this->_atomsById[atomId]);
                }
                return AddedPayload(out);
            }
            break; 

            case PayloadAlteration::Reset: {
                auto casted = (ResetPayload*)payload;
                return ResetPayload(this->_atomsById);
            }
            break;

            case PayloadAlteration::LayerChanged: {
                auto casted = (LayerChangedPayload*)payload;
            }
            break;

            case PayloadAlteration::TextChanged: {
                auto casted = (TextChangedPayload*)payload;
            }
            break;

            case PayloadAlteration::Rotated: {
                auto casted = (RotatedPayload*)payload;
            }
            break;

            case PayloadAlteration::Scaled: {
                auto casted = (ScaledPayload*)payload;
            }
            break;

            case PayloadAlteration::LockChanged: {
                auto casted = (LockingPayload*)payload;
            }
            break;
            
            case PayloadAlteration::VisibilityChanged: {
                auto casted = (VisibilityPayload*)payload;
            }
            break;
    }
}

//alter Scene
void AtomsStorage::_handlePayload(AlterationPayload* payload) { 
    
    //prevent circular payloads
    auto payloadSource = payload->source();
    if(payloadSource == this->_source) return;

    auto pType = payload->type();

    //on duplication
    if(pType == PayloadAlteration::Duplicated) {
        auto cPayload = (DuplicatedPayload*)payload;
        return this->_duplicateAtoms(cPayload->targetAtomIds());
    }

    if(pType == PayloadAlteration::Redone) return this->redo(); //on redo
    if(pType == PayloadAlteration::Undone) return this->undo(); //on undo

    //register
    this->_registerUndoPayload(payload);

    //on reset
    if(pType == PayloadAlteration::Reset) {
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
RPZAtom* AtomsStorage::_handlePayloadInternal(const PayloadAlteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) {

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
        case PayloadAlteration::LockChanged: {
            auto isLocked = atomAlteration.toBool();
            storedAtom->setLocked(isLocked);
        }
        break;
        
        // on changing visibility
        case PayloadAlteration::VisibilityChanged: {
            auto isHidden = atomAlteration.toBool();
            storedAtom->setHidden(isHidden);
        }
        break;

        //on move
        case PayloadAlteration::Moved: {
            auto position = atomAlteration.toPointF();
            storedAtom->setPos(position);
        }
        break;

        //on scaling
        case PayloadAlteration::Scaled: {
            auto scale = atomAlteration.toDouble();
            storedAtom->setScale(scale);
        }
        break;

        //on rotation
        case PayloadAlteration::Rotated: {
            auto deg = atomAlteration.toDouble();
            storedAtom->setRotation(deg);
        }
        break;

        //on resize
        case PayloadAlteration::LayerChanged: {
            auto layer = atomAlteration.toInt();
            storedAtom->setLayer(layer);
        }
        break;

        //on text change
        case PayloadAlteration::TextChanged: {
            auto text = atomAlteration.toString();
            storedAtom->setText(text);
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

        newAtom.setPos(currPos);

        newAtoms.insert(newAtom.id(), newAtom);
    }

    //new payload
    this->handleAlterationRequest(AddedPayload(newAtoms));
}


//////////////////
/* END ELEMENTS */
//////////////////