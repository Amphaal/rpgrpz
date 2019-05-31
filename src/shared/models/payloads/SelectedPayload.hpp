#pragma once

#include "base/MultipleTargetsPayload.hpp"

class SelectedPayload : public MultipleTargetsPayload {
    public:
        SelectedPayload(const QList<QUuid> &selectedAtomIds) : 
        MultipleTargetsPayload(AlterationPayload::Alteration::Selected, selectedAtomIds) {}
};