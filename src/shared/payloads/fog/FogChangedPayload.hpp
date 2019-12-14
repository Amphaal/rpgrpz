#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

#include "src/helpers/JSONSerializer.h"

class FogChangedPayload : public AlterationPayload {
    
    public:
        enum class ChangeType {
            Added,
            Removed,
            Reset
        };

        explicit FogChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        FogChangedPayload(const ChangeType &type, const QPainterPath &modifyingPath = QPainterPath()) : AlterationPayload(Payload::Alteration::FogModeChanged) {
            this->insert(QStringLiteral(u"ct"), (int)type);
            if(!modifyingPath.isEmpty()) this->insert(QStringLiteral(u"mp"), JSONSerializer::asBase64(modifyingPath));
        }
    
        ChangeType changeType() const {
            return (ChangeType)this->value(QStringLiteral(u"ct")).toInt();
        }

        const QPainterPath modifyingPath() const {
            auto rawPath = this->value("mp").toByteArray();
            return JSONSerializer::fromByteArray(rawPath);
        }
};