#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include "src/shared/models/RPZAssetMetadata.h"

class AssetChangedPayload : public AlterationPayload {
    
    public:
        AssetChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AssetChangedPayload(const RPZAssetMetadata &metadata) : AlterationPayload(PayloadAlteration::PA_AssetChanged) {
            this->insert("m", metadata);
        }
    
        RPZAssetMetadata assetMetadata() const {
            return this->value("m").toHash();
        }
};