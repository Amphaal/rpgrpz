#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class MetadataChangedPayload : public MultipleTargetsPayload {
    public:
        MetadataChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        
        //multiple targets / multiple params
        MetadataChangedPayload(
            const QVector<snowflake_uid> &targetedAtomIds,
            QHash<AtomParameter, QVariant> &changes
        ) : MultipleTargetsPayload(PayloadAlteration::PA_MetadataChanged, targetedAtomIds) {
            this->_setMetadataChanges(changes);
        }

        //multiple targets / single param
        MetadataChangedPayload(
            const QVector<snowflake_uid> &targetedAtomIds,
            const AtomParameter &param, 
            const QVariant &value
        ) : MultipleTargetsPayload(PayloadAlteration::PA_MetadataChanged, targetedAtomIds) {
            QHash<AtomParameter, QVariant> changes {{ param, value }};
            this->_setMetadataChanges(changes);
        }

        //single target / single param
        MetadataChangedPayload(
            snowflake_uid targetedId, 
            const AtomParameter &param, 
            const QVariant &value
        ) : MultipleTargetsPayload(PayloadAlteration::PA_MetadataChanged, QVector<snowflake_uid>({targetedId})) {
            QHash<AtomParameter, QVariant> changes {{ param, value }};
            this->_setMetadataChanges(changes);
        }

        static RPZAtom fromArgs(const QVariant &args) {
            RPZAtom out;

            auto base = args.toHash();           
            for (QVariantHash::iterator i = base.begin(); i != base.end(); ++i) {
                auto param = (AtomParameter)i.key().toInt();
                out.setMetadata(param, i.value(), false); //prevent autoremove
            }

            return out;
        }

    private:
        void _setMetadataChanges(QHash<AtomParameter, QVariant> &changes) {
            QVariantHash in;
            for (QHash<AtomParameter, QVariant>::iterator i = changes.begin(); i != changes.end(); ++i) {
                in.insert(QString::number((int)i.key()), i.value());
            }
            this->insert("args", in);
        }
};