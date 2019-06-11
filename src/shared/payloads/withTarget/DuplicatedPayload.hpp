#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

class DuplicatedPayload : public MultipleTargetsPayload {
    public:
        DuplicatedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        DuplicatedPayload(const QVector<snowflake_uid> &atomIdsToDuplicate) : MultipleTargetsPayload(PayloadAlteration::Duplicated, atomIdsToDuplicate) {}
};