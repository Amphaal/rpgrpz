#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class AddedPayload : public AtomsWielderPayload {
    public:
        AddedPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) { }
        AddedPayload(const RPZMap<RPZAtom> &atoms) : AtomsWielderPayload(PayloadAlteration::PA_Added, atoms) { }
        AddedPayload(const RPZAtom &atom) : AddedPayload(RPZMap<RPZAtom>(atom)) { }
        AddedPayload(const QList<RPZAtom> &atoms) : AddedPayload(RPZMap<RPZAtom>(atoms)) { }
        
};