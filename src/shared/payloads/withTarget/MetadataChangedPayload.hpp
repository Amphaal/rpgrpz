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
            return JSONSerializer::unserializeUpdates(this->args().toHash());
        }

    private:
        void _setMetadataChanges(const AtomUpdates &changes) {
            this->insert("args", 
                JSONSerializer::serializeUpdates(changes)
            );
        }
};