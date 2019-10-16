#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class ResetPayload : public AtomsWielderPayload {
    public:
        ResetPayload() {}
        ResetPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) {}
        ResetPayload(const RPZMap<RPZAtom> &atoms, const QSet<RPZAssetHash> &includedAssetIds) : AtomsWielderPayload(PayloadAlteration::PA_Reset, atoms, includedAssetIds) { }
};