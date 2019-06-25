#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class AddedPayload : public AtomsWielderPayload {
    public:
        AddedPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) { }
        AddedPayload(RPZMap<RPZAtom> atoms) : AtomsWielderPayload(PayloadAlteration::Added, atoms) { }
        AddedPayload(RPZAtom &atom) : AddedPayload(RPZMap<RPZAtom>(atom)) { }
        
};