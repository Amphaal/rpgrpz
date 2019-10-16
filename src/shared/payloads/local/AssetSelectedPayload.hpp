#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include <QPair>
#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/RPZToyMetadata.h"

class AssetSelectedPayload : public AlterationPayload {
    
    public:
        AssetSelectedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AssetSelectedPayload(const RPZToyMetadata &asset) : AlterationPayload(PayloadAlteration::PA_AssetSelected) {
            this->insert(QStringLiteral(u"as"), asset);
        }
    
        RPZToyMetadata selectedAsset() const {
            return RPZAtom(this->value(QStringLiteral(u"as")).toHash());
        }
};