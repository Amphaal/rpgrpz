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
    payload.changeSource(this->_source);

    emit atomsAlteredForLocal(payload);

    if(AlterationPayload::networkAlterations.contains(payload.type())) {
        emit atomsAlteredForNetwork(payload);
    }

} 

/////////////////
/* END NETWORK */
/////////////////

//////////////
/* ELEMENTS */
//////////////


//helper
void MapHint::alterScene(const QVariantHash &payload) {
    return this->_alterSceneGlobal(AlterationPayload(payload));
}

//alter Scene
void MapHint::_alterSceneGlobal(AlterationPayload &payload) { 

    //prevent circular payloads
    if(payload.source() == this->_source) return;

    //on reset
    auto pType = payload.type();
    if(pType == AlterationPayload::Alteration::Reset) {
        this->_selfElements.clear();
        this->_atomsById.clear();
        this->_foreignElementIdsByOwnerId.clear();
    }

    //handling
    auto alterations = payload.alterationByAtomId();
    for (QVariantHash::iterator i = alterations.begin(); i != alterations.end(); ++i) {
        this->_alterSceneInternal(pType, QUuid(i.key()), i.value());
    }

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
            auto ownerId = newAtom.owner().id();

            //bind to owners
            if(!ownerId.isNull()) {
                if(!this->_foreignElementIdsByOwnerId.contains(ownerId)) {
                    this->_foreignElementIdsByOwnerId.insert(ownerId, QSet<QUuid>());
                }
                this->_foreignElementIdsByOwnerId[ownerId].insert(targetedAtomId);
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
            storedAtom->metadata()->setPos(position);
        }
        break;

        //on resize
        case AlterationPayload::Alteration::LayerChanged: {
            auto layer = atomAlteration.toInt();
            storedAtom->metadata()->setLayer(layer);
        }
        break;
            
        //on removal
        case AlterationPayload::Alteration::Removed: {
            
            auto storedAtomOwnerId = storedAtom->owner().id();

            //unbind from owners
            if(!storedAtomOwnerId.isNull()) {
               this->_foreignElementIdsByOwnerId[storedAtomOwnerId].remove(targetedAtomId);
            } else {
                this->_selfElements.remove(targetedAtomId);
            }

            //update 
            this->_atomsById.remove(targetedAtomId);

        }
        break;

    }

    return storedAtom;
}

//////////////////
/* END ELEMENTS */
//////////////////