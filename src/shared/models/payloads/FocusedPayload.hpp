#pragma once

#include "base/AlterationPayload.hpp"

#include <QUuid>

class FocusedPayload : public AlterationPayload {
    public:
        FocusedPayload(QUuid &targetedAtomId) : AlterationPayload(AlterationPayload::Focused, AlterationPayload::Local) {
            this->_setTargetAtomId(targetedAtomId);
        }
    
        QUuid targetAtomId() {
            return this->value("id").toUuid();
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;

            out.insert(this->targetAtomId().toString(), QVariant());
            
            return out;
        }
    
    private:
        void _setTargetAtomId(const QUuid &targetAtomId) {
            (*this)["id"] = targetAtomId;
        }

};