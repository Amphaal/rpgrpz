#pragma once

#include "src/shared/models/toy/RPZAsset.hpp"
#include "src/shared/models/RPZAtom.h"

class RPZToy : public RPZAsset {
    public:
        RPZToy() {}
        explicit RPZToy(const QVariantHash &hash) : RPZAsset(hash) {}
        RPZToy(const RPZAsset &asset, const RPZAtom::Type &type = RPZAtom::Type::Undefined) : RPZAsset(asset) {
            this->insert(QStringLiteral(u"at"), (int)type);
        }

        RPZAtom::Type atomType() const {
            return (RPZAtom::Type)this->value(QStringLiteral(u"at")).toInt();
        }
};