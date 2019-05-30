#include "src/shared/models/entities/RPZAtom.hpp"

#include "base/AlterationPayload.hpp"

class AddedPayload : AlterationPayload {
    public:
        AddedPayload(const AlterationPayload::Source &source, RPZAtom &atom) : AlterationPayload(AlterationPayload::Alteration::Added, source) {
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