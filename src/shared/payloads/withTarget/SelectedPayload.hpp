#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

class SelectedPayload : public MultipleTargetsPayload {
    public:
        SelectedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        SelectedPayload(const QVector<RPZAtomId> &selectedRPZAtomIds) : MultipleTargetsPayload(PayloadAlteration::PA_Selected, selectedRPZAtomIds) {}
};