#pragma once

#include "src/shared/models/toy/RPZAsset.hpp"
#include "src/shared/models/RPZAtom.h"

class RPZToy : public RPZAsset {
    public:
        RPZToy() {}
        explicit RPZToy(const QVariantHash &hash) : RPZAsset(hash) {}
        RPZToy(const RPZAsset &asset, const RPZAtomType &type = RPZAtomType::Undefined) : RPZAsset(asset) {
            this->insert(QStringLiteral(u"at"), (int)type);
        }

        RPZAtomType atomType() const {
            return (RPZAtomType)this->value(QStringLiteral(u"at")).toInt();
        }
};