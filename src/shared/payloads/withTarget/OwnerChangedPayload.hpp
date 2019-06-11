#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

#include "src/shared/models/RPZUser.hpp"

class OwnerChangedPayload : public MultipleTargetsPayload {
    public:
        OwnerChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        OwnerChangedPayload(const QVector<snowflake_uid> &changedAtomIds, RPZUser &newOwner) : MultipleTargetsPayload(PayloadAlteration::OwnerChanged, changedAtomIds) {
            this->_setOwner(newOwner);
        }

        RPZUser owner() {
            return this->value("owner").toHash();
        }
    
    private:
        void _setOwner(RPZUser &newOwner) {
            (*this)["owner"] = newOwner;
        }

};