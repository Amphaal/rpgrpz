#pragma once

#include "base/AtomsWielderPayload.hpp"

class AddedPayload : public AtomsWielderPayload {
    public:
        AddedPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) { }
        AddedPayload(RPZMap<RPZAtom> &atoms) : AtomsWielderPayload(AlterationPayload::Alteration::Added, atoms) { }
        AddedPayload(RPZAtom &atom) : AddedPayload(RPZMap<RPZAtom>(atom)) { }
        
};