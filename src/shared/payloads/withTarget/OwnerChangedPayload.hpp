#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

#include "src/shared/models/RPZUser.h"

class OwnerChangedPayload : public MultipleTargetsPayload {
    public:
        OwnerChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        OwnerChangedPayload(const QVector<snowflake_uid> &changedAtomIds, const RPZUser &newOwner) : MultipleTargetsPayload(PayloadAlteration::PA_OwnerChanged, changedAtomIds) {
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