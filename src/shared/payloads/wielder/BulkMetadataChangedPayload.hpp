#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class BulkMetadataChangedPayload : public AtomsWielderPayload {
    public:
        BulkMetadataChangedPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) { }
        BulkMetadataChangedPayload(RPZMap<RPZAtom> &partialAtoms) : AtomsWielderPayload(PayloadAlteration::BulkMetadataChanged, partialAtoms) { }
};