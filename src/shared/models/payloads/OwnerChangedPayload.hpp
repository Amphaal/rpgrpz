#pragma once

#include "base/MultipleTargetsPayload.hpp"

#include "src/shared/models/entities/RPZUser.hpp"

class OwnerChangedPayload : public MultipleTargetsPayload {
    public:
        OwnerChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        OwnerChangedPayload(const QVector<snowflake_uid> &changedAtomIds, RPZUser &newOwner) : MultipleTargetsPayload(PayloadAlteration::OwnerChanged, changedAtomIds) {
            this->_setOwner(newOwner);
        }

        RPZUser owner() {
            return this->value("owner").toHash();
        }

        QVariantMap alterationByAtomId() override {
            
            auto list = this->targetAtomIds();
            auto owner = this->owner();

            QVariantMap out;
            for(auto &e : list) {
                out.insert(QString::number(e), owner);
            }

            return out;
        }
    
    private:
        void _setOwner(RPZUser &newOwner) {
            (*this)["owner"] = newOwner;
        }

};