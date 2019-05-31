#pragma once

#include "base/MultipleTargetsPayload.hpp"

class SelectedPayload : public MultipleTargetsPayload {
    public:
        SelectedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        SelectedPayload(const QVector<QUuid> &selectedAtomIds) : MultipleTargetsPayload(AlterationPayload::Alteration::Selected, selectedAtomIds) {}
};