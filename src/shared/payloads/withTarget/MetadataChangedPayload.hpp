#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class MetadataChangedPayload : public MultipleTargetsPayload {
    public:
    
        MetadataChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        MetadataChangedPayload(const QVector<RPZAtomId> &targetedRPZAtomIds, const AtomUpdates &changes) : 
            MultipleTargetsPayload(PayloadAlteration::PA_MetadataChanged, targetedRPZAtomIds) {
            
            this->_setMetadataChanges(changes);
            
        }

        AtomUpdates updates() const {
            
            AtomUpdates out;
            auto base = this->args().toHash();   

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
            this->insert("args", in);
        }
};