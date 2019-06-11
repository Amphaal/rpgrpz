#pragma once

#include "src/shared/models/RPZAtom.h"

#include "AlterationPayload.hpp"

class AtomsWielderPayload : public AlterationPayload {
    public:
        AtomsWielderPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomsWielderPayload(const PayloadAlteration &alteration, RPZMap<RPZAtom> &atoms) : AlterationPayload(alteration) {
            this->_setAddedAtoms(atoms);
        }
            
    RPZMap<RPZAtom> atoms() {
        RPZMap<RPZAtom> out;
        
        auto map = this->value("atoms").toMap();

        for(QVariantMap::iterator i = map.begin(); i != map.end(); ++i) { 
            out.insert(i.key().toULongLong(), i.value().toHash());
        }
        
        return out;
    }

    void updateEmptyUser(RPZUser &user) {
        
        auto atoms = this->atoms();
        for(auto &atom : atoms) {
            //override ownership on absent owner data
            if(atom.owner().isEmpty()) {
                atom.setOwnership(user);
            }
        }

        this->_setAddedAtoms(atoms);
    }

    private:
        void _setAddedAtoms(RPZMap<RPZAtom> &atoms) {
            QVariantMap list;
            for(auto &e : atoms) {
                list.insert(QString::number(e.id()), e);
            }
            (*this)["atoms"] = list;
        }
};