#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class AtomTemplateChangedPayload : public AlterationPayload {
    public:
        AtomTemplateChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomTemplateChangedPayload(const AtomUpdates &changes) : AlterationPayload(PayloadAlteration::AtomTemplateChanged) {
            this->_setMetadataChanges(changes);
        }

        AtomUpdates updates() const {
            
            AtomUpdates out;
            auto base = this->value(QStringLiteral(u"changes")).toHash();  

            for (auto i = base.begin(); i != base.end(); ++i) {
                auto param = (AtomParameter)i.key().toInt();
                out.insert(param, i.value());
            }

            return out;

        }

    private:
        void _setMetadataChanges(const AtomUpdates &changes) {
            
            QVariantHash in;
            
            for (auto i = changes.constBegin(); i != changes.constEnd(); ++i) {
                in.insert(QString::number((int)i.key()), i.value());
            }

            this->insert(QStringLiteral(u"changes"), in);

        }
};