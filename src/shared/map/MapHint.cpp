#include "MapHint.h"

MapHint::MapHint() {};

QVector<RPZAtom> MapHint::atoms() {
    return this->_atomsById.values().toVector();
}

//handle network and local evts emission
void MapHint::_emitAlteration(const AlterationPayload &payload) {

    emit atomsAlteredForLocal(payload);

    if(AlterationPayload::networkAlterations.contains(state) && !this->_preventNetworkAlterationEmission) {
        emit atomsAlteredForNetwork(state, elements);
    }

} 

QVariantHash MapHint::packageForNetworkSend(const AlterationPayload::Alteration &state, QVector<RPZAtom> &atoms) {
    return AlterationPayload(state, atoms);
}


/////////////////
/* END NETWORK */
/////////////////

//////////////
/* ELEMENTS */
//////////////

//register actions
void MapHint::_alterSceneInternal(const AlterationPayload::Alteration &alteration, RPZAtom &atom) {

    //get the Uuids
    auto elemId = atom.id();
    auto ownerId = atom.owner().id();

    //additionnal modifications
    switch(alteration) {

        //on addition
        case AlterationPayload::Alteration::Reset:
        case AlterationPayload::Alteration::Added: {
            
            //bind to owners
            if(!ownerId.isNull()) {
                if(!this->_foreignElementIdsByOwnerId.contains(ownerId)) {
                    this->_foreignElementIdsByOwnerId.insert(ownerId, QSet<QUuid>());
                }
                this->_foreignElementIdsByOwnerId[ownerId].insert(elemId);
            } else {
                this->_selfElements.insert(elemId);
            }

            //bind elem
            if(!this->_atomsById.contains(elemId)) {
                this->_atomsById.insert(elemId, atom);
            }
            
        }
        break;
        
        //on move / resize, update inner RPZAtom
        case AlterationPayload::Alteration::Moved:
        case AlterationPayload::Alteration::LayerChanged: {
            this->_atomsById[elemId] = atom;
        }
        break;
            
        //on removal
        case AlterationPayload::Alteration::Removed: {
            
            //unbind from owners
            if(!ownerId.isNull()) {
               this->_foreignElementIdsByOwnerId[ownerId].remove(elemId);
            } else {
                this->_selfElements.remove(elemId);
            }

            //update 
            this->_atomsById.remove(elemId);

        }
        break;

    }
}

//alter Scene
void MapHint::_alterSceneGlobal(const AlterationPayload::Alteration &alteration, QVector<RPZAtom> &atoms) { 

    //on reset
    if(alteration == AlterationPayload::Alteration::Reset) {
        this->_selfElements.clear();
        this->_atomsById.clear();
        this->_foreignElementIdsByOwnerId.clear();
    }

    //handling
    for(auto &atom : atoms) {
        this->_alterSceneInternal(alteration, atom);
    }
    //emit event
    this->_emitAlteration(alteration, atoms);
}

//helper
void MapHint::alterSceneFromAtom(const AlterationPayload::Alteration &alteration, RPZAtom &atom) {
    QVector<RPZAtom> list;
    list.append(atom);
    return this->_alterSceneGlobal(alteration, list);
}

//helper
void MapHint::alterSceneFromAtoms(const AlterationPayload::Alteration &alteration, QVector<RPZAtom> &atoms) {
    return this->_alterSceneGlobal(alteration, atoms);
}

//helper
void MapHint::alterSceneFromIds(const AlterationPayload::Alteration &alteration, const QVector<QUuid> &elementIds, QVariant &arg) {

    //apply new layer to atoms
    if(alteration == AlterationPayload::Alteration::LayerChanged) {
        auto layer = arg.toInt();
        for(auto &id : elementIds) {
            auto atom = this->_atomsById[id];
            atom.metadata()->setLayer(layer);
            this->_atomsById[id] = atom;
        }
    }

    return this->_alterSceneGlobal(alteration, this->_fetchAtoms(elementIds));
}

//helper
QVector<RPZAtom> MapHint::_fetchAtoms(const QVector<QUuid> &listToFetch) {
   QVector<RPZAtom> list;

    for(auto &e : listToFetch) {
        auto atom = this->_atomsById[e];
        list.append(atom);
    }

   return list; 
}

//////////////////
/* END ELEMENTS */
//////////////////