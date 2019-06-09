#pragma once

#include "base/AlterationPayload.hpp"

class UndonePayload : public AlterationPayload {
    public:
        UndonePayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        UndonePayload() : AlterationPayload(PayloadAlteration::Undone) {}
};