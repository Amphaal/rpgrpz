#pragma once

#include "src/shared/models/entities/RPZAtom.hpp"

#include "base/AlterationPayload.hpp"

class ResetPayload : public AlterationPayload {
    public:
        ResetPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        ResetPayload(QVector<RPZAtom> &atoms) : AlterationPayload(AlterationPayload::Alteration::Reset) {
            this->_setAddedAtoms(atoms);
        }
            
    QVector<RPZAtom> atoms() {
        QVector<RPZAtom> out;
        auto list = this->value("atoms").toList();
        for(auto &e : list) {
            out.append(e.toHash());
        }

        return out;
    }

    QVariantHash alterationByAtomId() override {
        QVariantHash out;

        for(auto &atom : this->atoms()) {
            out.insert(QString::number(atom.id()), atom);
        }
        
        return out;
    }

    private:
        void _setAddedAtoms(QVector<RPZAtom> &atoms) {
            QVariantList list;
            for(auto &e : atoms) {
                list.append(e);
            }
            (*this)["atoms"] = list;
        }
};