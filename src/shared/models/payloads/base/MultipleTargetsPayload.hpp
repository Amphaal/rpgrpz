#pragma once

#include "AlterationPayload.hpp"

#include <QList>
#include <QUuid>

class MultipleTargetsPayload : public AlterationPayload {
    public:
        MultipleTargetsPayload(
            const AlterationPayload::Alteration &alteration, 
            const AlterationPayload::Source &source,
            const QList<QUuid> &targetedAtomIds
        ) : AlterationPayload(alteration, source) {
            this->_setTargetAtomIds(targetedAtomIds);
        }
    
        QUuid targetAtomIds() {
            return this->value("ids").toUuid();
        }
    
    private:
        void _setTargetAtomIds(const QList<QUuid> &targetAtomIds) {
            (*this)["ids"] = QVariant::fromValue(targetAtomIds);
        }
};