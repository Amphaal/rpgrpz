#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class ResetPayload : public AtomsWielderPayload {
    public:
        ResetPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) {}
        ResetPayload(RPZMap<RPZAtom> &atoms) : AtomsWielderPayload(PayloadAlteration::Reset, atoms) { }
};