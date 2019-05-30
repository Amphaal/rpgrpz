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

    private:
        void _setAddedAtom(RPZAtom &atom) {
            (*this)["atom"] = atom;
        }
};