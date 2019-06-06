#pragma once

#include "base/MultipleTargetsPayload.hpp"

class LockingPayload : public MultipleTargetsPayload {
    public:
        LockingPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        LockingPayload(const QVector<snowflake_uid> &changedAtomIds, bool isLocked) : MultipleTargetsPayload(AlterationPayload::Alteration::LayerChanged, changedAtomIds) {
            this->_setLocked(isLocked);
        }

        bool locked() {
            return this->value("lck").toBool();
        }

        QVariantMap alterationByAtomId() override {
            
            auto list = this->targetAtomIds();
            auto isLocked = this->locked();

            QVariantMap out;
            for(auto &e : list) {
                out.insert(QString::number(e), isLocked);
            }

            return out;
        }
    
    private:
        void _setLocked(bool isLocked) {
            (*this)["lck"] = isLocked;
        }

};