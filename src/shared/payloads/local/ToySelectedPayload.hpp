#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include <QPair>
#include "src/shared/models/toy/RPZToy.hpp"

class ToySelectedPayload : public AlterationPayload {
    
    public:
        ToySelectedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        ToySelectedPayload(const RPZToy &toy) : AlterationPayload(Payload::Alteration::ToySelected) {
            this->insert(QStringLiteral(u"toy"), toy);
        }
    
        RPZToy selectedToy() const {
            return RPZToy(this->value(QStringLiteral(u"toy")).toHash());
        }

};