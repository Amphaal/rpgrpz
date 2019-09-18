#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

class RemovedPayload : public MultipleAtomTargetsPayload {
    public:
        RemovedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        RemovedPayload(const QVector<RPZAtomId> &RPZAtomIdsToRemove) : MultipleAtomTargetsPayload(PayloadAlteration::PA_Removed, RPZAtomIdsToRemove) {}
};