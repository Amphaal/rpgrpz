#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

#include "src/helpers/JSONSerializer.h"

class FogChangedPayload : public AlterationPayload {
    
    public:
        enum class ChangeType {
            Added,
            Removed
        };

        explicit FogChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        FogChangedPayload(const QPainterPath &modifyingPath, const ChangeType &type) : AlterationPayload(Payload::Alteration::FogModeChanged) {
            this->insert(QStringLiteral(u"mp"), JSONSerializer::asBase64(modifyingPath));
            this->insert(QStringLiteral(u"t"), (int)type);
        }
    
        ChangeType changeType() const {
            return (ChangeType)this->value(QStringLiteral(u"t")).toInt();
        }

        const QPainterPath modifyingPath() const {
            auto rawPath = this->value("mp").toByteArray();
            return JSONSerializer::fromByteArray(rawPath);
        }
};