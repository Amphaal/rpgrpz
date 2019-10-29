#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class MetadataChangedPayload : public MultipleAtomTargetsPayload {
    public:
        explicit MetadataChangedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        MetadataChangedPayload(const QVector<RPZAtom::Id> &targetedRPZAtomIds, const RPZAtom::Updates &changes) : MultipleAtomTargetsPayload(Payload::Alteration::MetadataChanged, targetedRPZAtomIds) {
            Q_ASSERT(targetedRPZAtomIds.count());
            Q_ASSERT(changes.count());

            this->_setMetadataChanges(changes);
            
        }

        RPZAtom::Updates updates() const {
            return RPZAtom::unserializeUpdates(this->args().toHash());
        }

    private:
        void _setMetadataChanges(const RPZAtom::Updates &changes) {
            this->insert(QStringLiteral(u"args"), 
                RPZAtom::serializeUpdates(changes)
            );
        }
};