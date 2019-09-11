#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include <QPair>
#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/RPZAssetMetadata.h"

class AssetSelectedPayload : public AlterationPayload {
    
    public:
        AssetSelectedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AssetSelectedPayload(const RPZAssetMetadata &asset) : AlterationPayload(PayloadAlteration::PA_AssetSelected) {
            this->insert("as", asset);
        }
    
        RPZAssetMetadata selectedAsset() const {
            return RPZAtom(this->value("as").toHash());
        }
};