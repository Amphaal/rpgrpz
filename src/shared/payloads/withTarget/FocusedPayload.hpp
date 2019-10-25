#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

class FocusedPayload : public MultipleAtomTargetsPayload {
    public:
        explicit FocusedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        FocusedPayload(RPZAtomId targetedRPZAtomId) : MultipleAtomTargetsPayload(Payload::Alteration::Focused, QVector<RPZAtomId>({targetedRPZAtomId})) {}
};