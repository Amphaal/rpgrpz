#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

class RedonePayload : public AlterationPayload {
    public:
        RedonePayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        RedonePayload() : AlterationPayload(PayloadAlteration::Redone) {}
};