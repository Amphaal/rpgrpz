#pragma once

#include "base/MultipleTargetsPayload.hpp"

class SelectedPayload : public MultipleTargetsPayload {
    public:
        SelectedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        SelectedPayload(const QVector<snowflake_uid> &selectedAtomIds) : MultipleTargetsPayload(AlterationPayload::Alteration::Selected, selectedAtomIds) {}
};