#pragma once

#include "base/MultipleTargetsPayload.hpp"

class VisibilityPayload : public MultipleTargetsPayload {
    public:
        VisibilityPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        VisibilityPayload(const QVector<snowflake_uid> &changedAtomIds, bool isHidden) : MultipleTargetsPayload(AlterationPayload::Alteration::VisibilityChanged, changedAtomIds) {
            this->_setHidden(isHidden);
        }

        bool hidden() {
            return this->value("hid").toBool();
        }

        QVariantMap alterationByAtomId() override {
            
            auto list = this->targetAtomIds();
            auto isHidden = this->hidden();

            QVariantMap out;
            for(auto &e : list) {
                out.insert(QString::number(e), isHidden);
            }

            return out;
        }
    
    private:
        void _setHidden(bool isHidden) {
            (*this)["hid"] = isHidden;
        }

};