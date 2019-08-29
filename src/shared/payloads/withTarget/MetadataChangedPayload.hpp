#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class MetadataChangedPayload : public MultipleTargetsPayload {
    public:
        MetadataChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        
        //multiple targets / multiple params
        MetadataChangedPayload(
            const QVector<RPZAtomId> &targetedRPZAtomIds,
            AtomUpdates &changes
        ) : MultipleTargetsPayload(PayloadAlteration::PA_MetadataChanged, targetedRPZAtomIds) {
            this->_setMetadataChanges(changes);
        }

        //multiple targets / single param
        MetadataChangedPayload(
            const QVector<RPZAtomId> &targetedRPZAtomIds,
            const AtomParameter &param, 
            const QVariant &value
        ) : MultipleTargetsPayload(PayloadAlteration::PA_MetadataChanged, targetedRPZAtomIds) {
            AtomUpdates changes {{ param, value }};
            this->_setMetadataChanges(changes);
        }

        //single target / single param
        MetadataChangedPayload(
            RPZAtomId targetedId, 
            const AtomParameter &param, 
            const QVariant &value
        ) : MultipleTargetsPayload(PayloadAlteration::PA_MetadataChanged, QVector<RPZAtomId>({targetedId})) {
            AtomUpdates changes {{ param, value }};
            this->_setMetadataChanges(changes);
        }

        AtomUpdates updates() const {
            return fromArgs(this->args());
        }

        static AtomUpdates fromArgs(const QVariant &args) {
            AtomUpdates out;
            auto base = args.toHash();   

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