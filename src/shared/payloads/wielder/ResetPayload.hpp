#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class ResetPayload : public AtomsWielderPayload {
    public:
        ResetPayload() {}
        ResetPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) {}
        ResetPayload(const MapDatabase &map) : AtomsWielderPayload(PayloadAlteration::Reset, map) { }
};