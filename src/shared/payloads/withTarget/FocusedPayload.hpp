#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

class FocusedPayload : public MultipleTargetsPayload {
    
    public:
        FocusedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        FocusedPayload(snowflake_uid targetedAtomId) : MultipleTargetsPayload(PayloadAlteration::PA_Focused, QVector<snowflake_uid>({targetedAtomId})) {}
};