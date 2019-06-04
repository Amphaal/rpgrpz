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

        QVariantHash alterationByAtomId() override {
            QVariantHash out;

            auto atom = this->atom();
            out.insert(atom.id().toString(), atom);
            
            return out;
        }

    private:
        void _setAddedAtom(RPZAtom &atom) {
            (*this)["atom"] = atom;
        }
};