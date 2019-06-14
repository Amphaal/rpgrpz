#pragma once

#include "src/shared/models/base/RPZMap.hpp"
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
            auto snowflakeId = i.key().toULongLong();
            RPZAtom atom(i.value().toHash());
            out.insert(snowflakeId, atom);
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
            for (RPZMap<RPZAtom>::iterator i = atoms.begin(); i != atoms.end(); ++i) {
                auto snowflakeAsStr = QString::number(i.key());
                auto maybePartialAtom = i.value();
                list.insert(snowflakeAsStr, maybePartialAtom);
            }
            this->insert("atoms", list);
        }
};