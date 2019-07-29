#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

class SelectedPayload : public MultipleTargetsPayload {
    public:
        SelectedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        SelectedPayload(const QVector<snowflake_uid> &selectedAtomIds) : MultipleTargetsPayload(PayloadAlteration::PA_Selected, selectedAtomIds) {}
};