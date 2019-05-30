#include "MapHint.h"

MapHint::MapHint() {};

QVector<RPZAtom> MapHint::atoms() {
    return this->_atomsById.values().toVector();
}

//handle network and local evts emission
void MapHint::_emitAlteration(const RPZAtom::Alteration &state, QVector<RPZAtom> &elements) {

    emit atomsAlteredForLocal(state, elements);

    if(RPZAtom::networkAlterations.contains(state) && !this->_preventNetworkAlterationEmission) {
        emit atomsAlteredForNetwork(state, elements);
    }

} 

QVariantHash MapHint::packageForNetworkSend(const RPZAtom::RPZAtom::Alteration &state, QVector<RPZAtom> &atoms) {
    AlterationPayload payload(state, atoms);
    return payload.toVariantHash();
}


/////////////////
/* END NETWORK */
/////////////////

//////////////
/* ELEMENTS */
//////////////

//register actions
void MapHint::_alterSceneInternal(const RPZAtom::Alteration &alteration, RPZAtom &atom) {

    //get the Uuids
    auto elemId = atom.id();
    auto ownerId = atom.owner().id();

    //additionnal modifications
    switch(alteration) {

        //on addition
        case RPZAtom::Alteration::Reset:
        case RPZAtom::Alteration::Added: {
            
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
        case RPZAtom::Alteration::Moved:
        case RPZAtom::Alteration::Resized:
        case RPZAtom::Alteration::LayerChange: {
            this->_atomsById[elemId] = atom;
        }
        break;
            
        //on removal
        case RPZAtom::Alteration::Removed: {
            
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
void MapHint::_alterSceneGlobal(const RPZAtom::Alteration &alteration, QVector<RPZAtom> &atoms) { 

    //on reset
    if(alteration == RPZAtom::Alteration::Reset) {
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
void MapHint::alterSceneFromAtom(const RPZAtom::Alteration &alteration, RPZAtom &atom) {
    QVector<RPZAtom> list;
    list.append(atom);
    return this->_alterSceneGlobal(alteration, list);
}

//helper
void MapHint::alterSceneFromAtoms(const RPZAtom::Alteration &alteration, QVector<RPZAtom> &atoms) {
    return this->_alterSceneGlobal(alteration, atoms);
}

//helper
void MapHint::alterSceneFromIds(const RPZAtom::Alteration &alteration, const QVector<QUuid> &elementIds, QVariant &arg) {

    //apply new layer to atoms
    if(alteration == RPZAtom::Alteration::LayerChange) {
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
QVector<RPZAtom> MapHint::_fetchAtoms(const QVector<QUuid> &listToFetch) const {
   QVector<RPZAtom> list;

    for(auto &e : listToFetch) {
        list.append(this->_atomsById[e]);
    }

   return list; 
}

//////////////////
/* END ELEMENTS */
//////////////////