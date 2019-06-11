#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class MetadataChangedPayload : public MultipleTargetsPayload {
    public:
        MetadataChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        MetadataChangedPayload(
            const QVector<snowflake_uid> &targetedAtomIds,
            QHash<RPZAtom::Parameters, QVariant> &changes
        ) : MultipleTargetsPayload(PayloadAlteration::MetadataChanged, targetedAtomIds) {
            this->_setMetadataChanges(changes);
        }

        QHash<RPZAtom::Parameters, QVariant> changes() {
            QHash<RPZAtom::Parameters, QVariant> out;

            auto base = this->value("changes").toHash();           
            for (QVariantHash::iterator i = base.begin(); i != base.end(); ++i) {
                out.insert((RPZAtom::Parameters)i.key().toInt(), i.value());
            }

            return out;
        }

    private:
        void _setMetadataChanges(QHash<RPZAtom::Parameters, QVariant> &changes) {
            QVariantHash in;
            for (QHash<RPZAtom::Parameters, QVariant>::iterator i = changes.begin(); i != changes.end(); ++i) {
                in.insert(QString::number(i.key()), i.value());
            }
            (*this)["changes"] = in;
        }
};