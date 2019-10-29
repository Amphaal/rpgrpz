#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class AtomTemplateChangedPayload : public AlterationPayload {
    public:
        explicit AtomTemplateChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomTemplateChangedPayload(const RPZAtom::Updates &changes) : AlterationPayload(Payload::Alteration::AtomTemplateChanged) {
            this->_setMetadataChanges(changes);
        }

        RPZAtom::Updates updates() const {
            
            RPZAtom::Updates out;
            auto base = this->value(QStringLiteral(u"changes")).toHash();  

            for (auto i = base.begin(); i != base.end(); ++i) {
                auto param = (RPZAtom::Parameter)i.key().toInt();
                out.insert(param, i.value());
            }

            return out;

        }

    private:
        void _setMetadataChanges(const RPZAtom::Updates &changes) {
            
            QVariantHash in;
            
            for (auto i = changes.constBegin(); i != changes.constEnd(); ++i) {
                in.insert(QString::number((int)i.key()), i.value());
            }

            this->insert(QStringLiteral(u"changes"), in);

        }
};