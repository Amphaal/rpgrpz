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
            QHash<RPZAtom::Parameters, QVariant> &changes
        ) : MultipleTargetsPayload(PayloadAlteration::MetadataChanged, targetedAtomIds) {
            this->_setMetadataChanges(changes);
        }

        //multiple targets / single param
        MetadataChangedPayload(
            const QVector<snowflake_uid> &targetedAtomIds,
            const RPZAtom::Parameters &param, 
            const QVariant &value
        ) : MultipleTargetsPayload(PayloadAlteration::MetadataChanged, targetedAtomIds) {
            QHash<RPZAtom::Parameters, QVariant> changes {{ param, value }};
            this->_setMetadataChanges(changes);
        }

        //single target / single param
        MetadataChangedPayload(
            const snowflake_uid &targetedId, 
            const RPZAtom::Parameters &param, 
            const QVariant &value
        ) : MultipleTargetsPayload(PayloadAlteration::MetadataChanged, QVector<snowflake_uid>({targetedId})) {
            QHash<RPZAtom::Parameters, QVariant> changes {{ param, value }};
            this->_setMetadataChanges(changes);
        }

        static RPZAtom fromArgs(const QVariant &args) {
            RPZAtom out;

            auto base = args.toHash();           
            for (QVariantHash::iterator i = base.begin(); i != base.end(); ++i) {
                auto param = (RPZAtom::Parameters)i.key().toInt();
                out.setMetadata(param, i.value());
            }

            return out;
        }

    private:
        void _setMetadataChanges(QHash<RPZAtom::Parameters, QVariant> &changes) {
            QVariantHash in;
            for (QHash<RPZAtom::Parameters, QVariant>::iterator i = changes.begin(); i != changes.end(); ++i) {
                in.insert(QString::number(i.key()), i.value());
            }
            this->insert("args", in);
        }
};