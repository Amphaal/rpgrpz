#pragma once

#include "base/AtomsWielderPayload.hpp"

class ResetPayload : public AtomsWielderPayload {
    public:
        ResetPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) {}
        ResetPayload(RPZMap<RPZAtom> &atoms) : AtomsWielderPayload(AlterationPayload::Alteration::Reset, atoms) { }
};