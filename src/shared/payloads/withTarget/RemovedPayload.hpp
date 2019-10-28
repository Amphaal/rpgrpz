#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

class RemovedPayload : public MultipleAtomTargetsPayload {
    public:
        explicit RemovedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        RemovedPayload(const QVector<RPZAtomId> &RPZAtomIdsToRemove) : MultipleAtomTargetsPayload(Payload::Alteration::Removed, RPZAtomIdsToRemove) {
            Q_ASSERT(RPZAtomIdsToRemove.count());
            Q_ASSERT(RPZAtomIdsToRemove.first());
        }
};