#pragma once

#include "AlterationPayload.hpp"

#include <QList>

class MultipleTargetsPayload : public AlterationPayload {
    public:

        MultipleTargetsPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        MultipleTargetsPayload(const AlterationPayload::Alteration &alteration, const QVector<snowflake_uid> &targetedAtomIds) : AlterationPayload(alteration) {
            this->_setTargetAtomIds(targetedAtomIds);
        }
    
        QVector<snowflake_uid> targetAtomIds() {
            
            auto list = this->value("ids").toList();

            QVector<snowflake_uid> out;
            for(auto &e : list) {
                out.append(e.toULongLong());
            }
            
            return out;
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;
            auto list = this->targetAtomIds();
            for(auto &e : list) {
                out.insert(QString::number(e), QVariant());
            }
            return out;
        }
    
    private:
        void _setTargetAtomIds(const QVector<snowflake_uid> &targetAtomIds) {
            QVariantList cast;
            for(auto &id : targetAtomIds) {
                cast.append(
                    QString::number(id)
                );
            }
            (*this)["ids"] = cast;
        }
};