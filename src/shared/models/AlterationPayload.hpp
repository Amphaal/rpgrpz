#pragma once

#include "src/shared/map/MapHint.h"
#include "RPZAtom.hpp"

#include "../Serializable.hpp"

class AlterationPayload : public Serializable {
    public:
        AlterationPayload(const RPZAtom::Alteration &alteration, QVector<RPZAtom> &atoms) : 
        Serializable(NULL), 
        _alteration(alteration),
        _atoms(atoms) { };

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("state", (int)this->_alteration);

            QVariantList atoms;
            for(auto &i : this->_atoms) {
               atoms.append(i.toVariantHash());
            }

            out.insert("atoms", atoms);

            return out;
        };

        QVector<RPZAtom>* atoms() { return &this->_atoms; };
        RPZAtom::Alteration alteration() { return this->_alteration; };

        static AlterationPayload fromVariantHash(const QVariantHash &data) {
            
            auto state = (RPZAtom::Alteration)data["state"].toInt();
            
            QVector<RPZAtom> atoms;
            for(auto &i : data["atoms"].toList()) {
                atoms.append(RPZAtom::fromVariantHash(i.toHash()));
            }
            
            return AlterationPayload(state, atoms);
        };


    private:
        RPZAtom::Alteration _alteration;
        QVector<RPZAtom> _atoms;
};