#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include <QPair>
#include "src/shared/models/RPZAtom.h"


class AtomTemplateSelectedPayload : public AlterationPayload {
    
    public:
        AtomTemplateSelectedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomTemplateSelectedPayload(const RPZAtom* ref) : AlterationPayload(PayloadAlteration::PA_AtomTemplateSelected) {
            this->insert("ref", QVariant::fromValue<RPZAtom*>((RPZAtom*)ref));
        }
    
        const RPZAtom* selectedTemplate() const {
            return this->value("ref").value<RPZAtom*>();
        }
};