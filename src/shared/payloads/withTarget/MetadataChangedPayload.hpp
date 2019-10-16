#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class MetadataChangedPayload : public MultipleAtomTargetsPayload {
    public:
        MetadataChangedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        MetadataChangedPayload(const QVector<RPZAtomId> &targetedRPZAtomIds, const AtomUpdates &changes) : 
            MultipleAtomTargetsPayload(PayloadAlteration::PA_MetadataChanged, targetedRPZAtomIds) {
            
            this->_setMetadataChanges(changes);
            
        }

        AtomUpdates updates() const {
            return JSONSerializer::unserializeUpdates(this->args().toHash());
        }

    private:
        void _setMetadataChanges(const AtomUpdates &changes) {
            this->insert(QStringLiteral(u"args"), 
                JSONSerializer::serializeUpdates(changes)
            );
        }
};