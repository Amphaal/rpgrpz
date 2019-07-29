#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

class UndonePayload : public AlterationPayload {
    public:
        UndonePayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        UndonePayload() : AlterationPayload(PayloadAlteration::PA_Undone) {}
};