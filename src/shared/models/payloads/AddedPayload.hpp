#pragma once

#include "src/shared/models/entities/RPZAtom.hpp"

#include "base/AlterationPayload.hpp"

class AddedPayload : public AlterationPayload {
    public:

        AddedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AddedPayload(RPZAtom &atom) : AlterationPayload(AlterationPayload::Alteration::Added) {
            this->_setAddedAtom(atom);
        }
    
        RPZAtom atom() {
            return this->value("atom").toHash();
        }

        QVariantMap alterationByAtomId() override {
            QVariantMap out;

            auto atom = this->atom();
            out.insert(QString::number(atom.id()), atom);
            
            return out;
        }

    private:
        void _setAddedAtom(RPZAtom &atom) {
            (*this)["atom"] = atom;
        }
};