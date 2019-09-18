#pragma once

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

#include "src/shared/models/RPZUser.h"

class OwnerChangedPayload : public MultipleAtomTargetsPayload {
    public:
        OwnerChangedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        OwnerChangedPayload(const QVector<RPZAtomId> &changedRPZAtomIds, const RPZUser &newOwner) : MultipleAtomTargetsPayload(PayloadAlteration::PA_OwnerChanged, changedRPZAtomIds) {
            this->_setOwner(newOwner);
        }

        RPZUser newOwner() const {
            return RPZUser(this->args().toHash());
        }
    
    private:
        void _setOwner(const RPZUser &newOwner) {
            this->insert("args", newOwner);
        }

};