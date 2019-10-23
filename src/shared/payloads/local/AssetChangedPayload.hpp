#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include "src/shared/models/toy/RPZToyMetadata.h"

class AssetChangedPayload : public AlterationPayload {
    
    public:
        AssetChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AssetChangedPayload(const RPZToyMetadata &metadata) : AlterationPayload(PayloadAlteration::AssetChanged) {
            this->insert(QStringLiteral(u"m"), metadata);
        }
    
        RPZToyMetadata assetMetadata() const {
            return this->value(QStringLiteral(u"m")).toHash();
        }
};