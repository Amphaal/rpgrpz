#pragma once

#include "base/MultipleTargetsPayload.hpp"

class RemovedPayload : public MultipleTargetsPayload {
    public:
        RemovedPayload(const AlterationPayload::Source &source, const QList<QUuid> &atomIdsToRemove) : 
        MultipleTargetsPayload(AlterationPayload::Alteration::Removed, source, atomIdsToRemove) {}
};