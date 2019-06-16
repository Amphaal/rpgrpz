#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

class FocusedPayload : public MultipleTargetsPayload {
    
    public:
        FocusedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        FocusedPayload(const snowflake_uid &targetedAtomId) : MultipleTargetsPayload(PayloadAlteration::Focused, QVector<snowflake_uid>({targetedAtomId})) {}
};