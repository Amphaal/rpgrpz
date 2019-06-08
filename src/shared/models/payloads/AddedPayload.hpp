#pragma once

#include "src/shared/models/entities/RPZAtom.hpp"

#include "base/AlterationPayload.hpp"

class AddedPayload : public AlterationPayload {
    public:
        AddedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AddedPayload(RPZMap<RPZAtom> &atoms) : AlterationPayload(AlterationPayload::Alteration::Added) {
            this->_setAddedAtoms(atoms);
        }
        AddedPayload(RPZAtom &atom) : AddedPayload(RPZMap<RPZAtom>(atom)) { }

            
    RPZMap<RPZAtom> atoms() {
        RPZMap<RPZAtom> out;
        
        auto map = this->value("atoms").toMap();

        for(QVariantMap::iterator i = map.begin(); i != map.end(); ++i) { 
            out.insert(i.key().toULongLong(), i.value().toHash());
        }
        
        return out;
    }

    QVariantMap alterationByAtomId() override {
        QVariantMap out;
     
        auto list = this->atoms();
        for(auto &atom : list) {
            out.insert(QString::number(atom.id()), atom);
        }

        return out;
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