#pragma once

#include "base/AlterationPayload.hpp"

class FocusedPayload : public AlterationPayload {
    
    public:
        FocusedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        FocusedPayload(const snowflake_uid &targetedAtomId) : AlterationPayload(AlterationPayload::Focused) {
            this->_setTargetAtomId(targetedAtomId);
        }
    
        snowflake_uid targetAtomId() {
            return this->value("id").toULongLong();
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;

            out.insert(QString::number(this->targetAtomId()), QVariant());
            
            return out;
        }
    
    private:
        void _setTargetAtomId(const snowflake_uid &targetAtomId) {
            (*this)["id"] = QString::number(targetAtomId);
        }
};