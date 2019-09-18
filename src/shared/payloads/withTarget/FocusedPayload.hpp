#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

class FocusedPayload : public MultipleAtomTargetsPayload {
    public:
        FocusedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        FocusedPayload(RPZAtomId targetedRPZAtomId) : MultipleAtomTargetsPayload(PayloadAlteration::PA_Focused, QVector<RPZAtomId>({targetedRPZAtomId})) {}
};