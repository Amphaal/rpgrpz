#pragma once

#include "base/AlterationPayload.hpp"

class RedonePayload : public AlterationPayload {
    public:
        RedonePayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        RedonePayload() : AlterationPayload(PayloadAlteration::Redone) {}
};