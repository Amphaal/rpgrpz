#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include "src/shared/models/toy/RPZAsset.hpp"

class AssetChangedPayload : public AlterationPayload {
    
    public:
        AssetChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AssetChangedPayload(const RPZAsset &metadata) : AlterationPayload(PayloadAlteration::AssetChanged) {
            this->insert(QStringLiteral(u"m"), metadata);
        }
    
        RPZAsset assetMetadata() const {
            return this->value(QStringLiteral(u"m")).toHash();
        }
};