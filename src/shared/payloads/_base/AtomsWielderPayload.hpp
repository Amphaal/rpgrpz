#pragma once

#include "src/shared/models/base/RPZMap.hpp"
#include "src/shared/models/RPZAtom.h"

#include "AlterationPayload.hpp"

class AtomsWielderPayload : public AlterationPayload {
    public:
        AtomsWielderPayload() {}
        AtomsWielderPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomsWielderPayload(const PayloadAlteration &alteration, const RPZMap<RPZAtom> &atoms) : AlterationPayload(alteration) {
            this->_setAddedAtoms(atoms);
        }
            
    RPZMap<RPZAtom> atoms() const {
        RPZMap<RPZAtom> out;
        
        auto map = this->value("atoms").toMap();

        for(auto i = map.begin(); i != map.end(); ++i) {    
            auto snowflakeId = i.key().toULongLong();
            RPZAtom atom(i.value().toHash());
            out.insert(snowflakeId, atom);
        }
        
        return out;
    }

    QVector<RPZAtomId> updateEmptyUser(const RPZUser &user) {
        
        auto atoms = this->atoms();

        QVector<RPZAtomId> updatedRPZAtomIds;

        for(auto &atom : atoms) {

            auto currentOwner = atom.owner();

            //override ownership on absent owner data
            if(currentOwner.isEmpty()) {

                atom.setOwnership(user);
                updatedRPZAtomIds.append(atom.id());

            }

        }

        this->_setAddedAtoms(atoms);

        return updatedRPZAtomIds;
    }

    private:
        void _setAddedAtoms(const RPZMap<RPZAtom> &atoms) {
            QVariantMap list;
            for (RPZMap<RPZAtom>::const_iterator i = atoms.constBegin(); i != atoms.constEnd(); ++i) {
                auto snowflakeAsStr = QString::number(i.key());
                auto maybePartialAtom = i.value();
                list.insert(snowflakeAsStr, maybePartialAtom);
            }
            this->insert("atoms", list);
        }
};