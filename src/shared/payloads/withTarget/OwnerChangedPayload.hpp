#pragma once

#include "src/shared/payloads/_base/MultipleTargetsPayload.hpp"

#include "src/shared/models/RPZUser.hpp"

class OwnerChangedPayload : public MultipleTargetsPayload {
    public:
        OwnerChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        OwnerChangedPayload(const QVector<snowflake_uid> &changedAtomIds, RPZUser &newOwner) : MultipleTargetsPayload(PayloadAlteration::OwnerChanged, changedAtomIds) {
            this->_setOwner(newOwner);
        }

        static RPZUser fromArgs(const QVariant &args) {
            return RPZUser(args.toHash());
        }
    
    private:
        void _setOwner(RPZUser &newOwner) {
            this->insert("args", newOwner);
        }

};