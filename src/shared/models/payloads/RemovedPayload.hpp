#pragma once

#include "base/MultipleTargetsPayload.hpp"

class RemovedPayload : public MultipleTargetsPayload {
    public:
        RemovedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        RemovedPayload(const QVector<QUuid> &atomIdsToRemove) : MultipleTargetsPayload(AlterationPayload::Alteration::Removed, atomIdsToRemove) {}
};