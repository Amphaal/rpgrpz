#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

class FocusedPayload : public MultipleAtomTargetsPayload {
    public:
        explicit FocusedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        FocusedPayload(RPZAtom::Id targetedRPZAtomId) : MultipleAtomTargetsPayload(Payload::Alteration::Focused, QList<RPZAtom::Id>({targetedRPZAtomId})) {
            Q_ASSERT(targetedRPZAtomId);
        }
};