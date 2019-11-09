#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

class RemovedPayload : public MultipleAtomTargetsPayload {
    public:
        explicit RemovedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        RemovedPayload(const QList<RPZAtom::Id> &atomIdsToRemove) : MultipleAtomTargetsPayload(Payload::Alteration::Removed, atomIdsToRemove) {
            Q_ASSERT(atomIdsToRemove.count() != 0);
            Q_ASSERT(atomIdsToRemove.first() != 0);
        }
};