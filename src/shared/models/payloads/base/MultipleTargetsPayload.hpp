#pragma once

#include "AlterationPayload.hpp"

#include <QList>
#include <QUuid>

class MultipleTargetsPayload : public AlterationPayload {
    public:
        MultipleTargetsPayload(
            const AlterationPayload::Alteration &alteration, 
            const QList<QUuid> &targetedAtomIds
        ) : AlterationPayload(alteration) {
            this->_setTargetAtomIds(targetedAtomIds);
        }
    
        QList<QUuid> targetAtomIds() {
            QList<QUuid> out;
            auto list = this->value("ids").toList();
            for(auto &e : list) {
                list.append(e.toUuid());
            }
            return out;
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;
            auto list = this->targetAtomIds();
            for(auto &e : list) {
                out.insert(e.toString(), QVariant());
            }
            return out;
        }
    
    private:
        void _setTargetAtomIds(const QList<QUuid> &targetAtomIds) {
            (*this)["ids"] = QVariant::fromValue(targetAtomIds);
        }
};