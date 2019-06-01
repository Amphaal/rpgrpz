#include "MapHint.h"


MapHint::MapHint(const AlterationPayload::Source &boundSource) : _source(boundSource) {};

AlterationPayload::Source MapHint::source() {
    return this->_source;
}

QVector<RPZAtom> MapHint::atoms() {
    return this->_atomsById.values().toVector();
}

//handle network and local evts emission
void MapHint::_emitAlteration(AlterationPayload &payload) {

    //define source of payload
    auto source = payload.source();
    if(source == AlterationPayload::Source::Network) return; //prevent resending network payloay
    if(source == AlterationPayload::Source::Undefined) payload.changeSource(this->_source); //inner payload, apply own source

    emit atomsAltered(payload);
} 

/////////////////
/* END NETWORK */
/////////////////

//////////////
/* ELEMENTS */
//////////////


//helper
void MapHint::alterScene(QVariantHash &payload) {
    return this->_alterSceneGlobal(AlterationPayload(payload));
}

//alter Scene
void MapHint::_alterSceneGlobal(AlterationPayload &payload) { 
    
    //prevent circular payloads
    auto payloadSource = payload.source();
    if(payloadSource == this->_source) return;

    //on reset
    auto pType = payload.type();
    if(pType == AlterationPayload::Alteration::Reset) {
        this->_selfElements.clear();
        this->_atomsById.clear();
        this->_foreignElementIdsByOwnerId.clear();
    }

    //handling
    auto aCasted = Payload::autoCast(payload);
    auto alterations = aCasted->alterationByAtomId();
    for (QVariantHash::iterator i = alterations.begin(); i != alterations.end(); ++i) {
        this->_alterSceneInternal(pType, QUuid(i.key()), i.value());
    }
    delete aCasted;

    //emit event
    this->_emitAlteration(payload);
}

//register actions
RPZAtom* MapHint::_alterSceneInternal(const AlterationPayload::Alteration &type, QUuid &targetedAtomId, QVariant &atomAlteration) {

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
            if(!owner.isEmpty()) {
                this->_foreignElementIdsByOwnerId[owner.id()].insert(targetedAtomId);
            } else {
                this->_selfElements.insert(targetedAtomId);
            }

            //bind elem
            this->_atomsById.insert(targetedAtomId, newAtom);
            
            //replace for return
            storedAtom = &this->_atomsById[targetedAtomId];
            
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

        //on resize
        case AlterationPayload::Alteration::LayerChanged: {
            auto layer = atomAlteration.toInt();
            auto mdata = storedAtom->metadata();
            mdata.setLayer(layer);
            storedAtom->setMetadata(mdata);
        }
        break;
            
        //on removal
        case AlterationPayload::Alteration::Removed: {
            
            auto storedAtomOwner = storedAtom->owner();

            //unbind from owners
            if(!storedAtomOwner.isEmpty()) {
               this->_foreignElementIdsByOwnerId[storedAtomOwner.id()].remove(targetedAtomId);
            } else {
                this->_selfElements.remove(targetedAtomId);
            }

            //update 
            delete storedAtom->graphicsItem();
            this->_atomsById.remove(targetedAtomId); 
            storedAtom = nullptr;

        }
        break;

    }

    return storedAtom;
}

//////////////////
/* END ELEMENTS */
//////////////////