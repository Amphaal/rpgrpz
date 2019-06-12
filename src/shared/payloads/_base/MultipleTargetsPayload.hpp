#pragma once

#include "AlterationPayload.hpp"

#include <QList>

class MultipleTargetsPayload : public AlterationPayload {
    public:

        MultipleTargetsPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        MultipleTargetsPayload(const PayloadAlteration &alteration, const QVector<snowflake_uid> &targetedAtomIds) : AlterationPayload(alteration) {
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

        virtual QVariant args() {
            return this->value("args");
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