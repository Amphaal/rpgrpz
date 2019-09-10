#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include <QPair>
#include "src/shared/models/RPZAtom.h"

class AtomTemplateSelectedPayload : public AlterationPayload {
    
    public:
        AtomTemplateSelectedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomTemplateSelectedPayload(const RPZAtom &templated) : AlterationPayload(PayloadAlteration::PA_AtomTemplateSelected) {
            this->insert("t", templated);
        }
    
        RPZAtom selectedTemplate() const {
            return RPZAtom(this->value("t").toHash());
        }
};