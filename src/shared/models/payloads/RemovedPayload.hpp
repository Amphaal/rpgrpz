#pragma once

#include "base/MultipleTargetsPayload.hpp"

class RemovedPayload : public MultipleTargetsPayload {
    public:
        RemovedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        RemovedPayload(const QVector<snowflake_uid> &atomIdsToRemove) : MultipleTargetsPayload(AlterationPayload::Alteration::Removed, atomIdsToRemove) {}
};