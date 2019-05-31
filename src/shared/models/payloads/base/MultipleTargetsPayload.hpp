#pragma once

#include "AlterationPayload.hpp"

#include <QList>
#include <QUuid>

class MultipleTargetsPayload : public AlterationPayload {
    public:

        MultipleTargetsPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        MultipleTargetsPayload(const AlterationPayload::Alteration &alteration, const QVector<QUuid> &targetedAtomIds) : AlterationPayload(alteration) {
            this->_setTargetAtomIds(targetedAtomIds);
        }
    
        QVector<QUuid> targetAtomIds() {
            
            auto list = this->value("ids").toList();

            QVector<QUuid> out;
            for(auto &e : list) {
                out.append(e.toUuid());
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
        void _setTargetAtomIds(const QVector<QUuid> &targetAtomIds) {
            QVariantList cast;
            for(auto &id : targetAtomIds) {
                cast.append(id);
            }
            (*this)["ids"] = cast;
        }
};