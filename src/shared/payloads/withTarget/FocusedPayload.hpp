#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

class FocusedPayload : public MultipleTargetsPayload {
    
    public:
        FocusedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        FocusedPayload(RPZAtomId targetedRPZAtomId) : MultipleTargetsPayload(PayloadAlteration::PA_Focused, QVector<RPZAtomId>({targetedRPZAtomId})) {}
};