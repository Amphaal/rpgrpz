#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

class SelectedPayload : public MultipleAtomTargetsPayload {
    public:
        SelectedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        SelectedPayload(const QVector<RPZAtomId> &selectedRPZAtomIds) : MultipleAtomTargetsPayload(Payload::Alteration::Selected, selectedRPZAtomIds) {}
};