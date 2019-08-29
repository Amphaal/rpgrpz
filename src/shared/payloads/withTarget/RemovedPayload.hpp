#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

class RemovedPayload : public MultipleTargetsPayload {
    public:
        RemovedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        RemovedPayload(const QVector<RPZAtomId> &RPZAtomIdsToRemove) : MultipleTargetsPayload(PayloadAlteration::PA_Removed, RPZAtomIdsToRemove) {}
};