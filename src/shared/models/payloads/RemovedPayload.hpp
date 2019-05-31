#pragma once

#include "base/MultipleTargetsPayload.hpp"

class RemovedPayload : public MultipleTargetsPayload {
    public:
        RemovedPayload(const QList<QUuid> &atomIdsToRemove) : 
        MultipleTargetsPayload(AlterationPayload::Alteration::Removed, atomIdsToRemove) {}
};