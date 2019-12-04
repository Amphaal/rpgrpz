#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

#include "src/shared/models/RPZFogParams.hpp"

class FogModeChangedPayload : public AlterationPayload {
    
    public:
        explicit FogModeChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        FogModeChangedPayload(const RPZFogParams::Mode &mode) : AlterationPayload(Payload::Alteration::FogModeChanged) {
            this->insert(QStringLiteral(u"m"), (int)mode);
        }
    
        RPZFogParams::Mode mode() const {
            return (RPZFogParams::Mode)this->value(QStringLiteral(u"m")).toInt();
        }
};