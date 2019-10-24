#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include <QPair>
#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/toy/RPZAsset.hpp"

class AssetSelectedPayload : public AlterationPayload {
    
    public:
        AssetSelectedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AssetSelectedPayload(const RPZAsset &asset) : AlterationPayload(PayloadAlteration::AssetSelected) {
            this->insert(QStringLiteral(u"as"), asset);
        }
    
        RPZAsset selectedAsset() const {
            return RPZAtom(this->value(QStringLiteral(u"as")).toHash());
        }
};