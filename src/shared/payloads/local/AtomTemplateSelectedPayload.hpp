#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include <QPair>
#include "src/shared/models/RPZAtom.h"


class AtomTemplateSelectedPayload : public AlterationPayload {
    
    public:
        AtomTemplateSelectedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomTemplateSelectedPayload(const RPZAtom &templateAtom) : AlterationPayload(PayloadAlteration::PA_AtomTemplateSelected) {
            this->insert(QStringLiteral(u"templ"), QVariant::fromValue<RPZAtom>(templateAtom));
        }
    
        const RPZAtom selectedTemplate() const {
            return this->value(QStringLiteral(u"templ")).value<RPZAtom>();
        }
};