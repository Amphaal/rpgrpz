#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include <QPair>
#include "src/shared/models/RPZAtom.h"

class AtomTemplateChangedPayload : public AlterationPayload {
    
    public:
        AtomTemplateChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomTemplateChangedPayload(const AtomParameter &param, const QVariant &value) : AlterationPayload(PayloadAlteration::PA_AtomTemplateChanged) {
            this->insert("p", param);
            this->insert("v", value);
        }
    
        AtomUpdates updates() const {
            return AtomUpdates {
                { (AtomParameter)this->value("p").toInt(), this->value("v") }
            };
        }
};