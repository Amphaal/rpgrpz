#pragma once

#include "AlterationPayload.hpp"

#include <QList>

class MultipleAtomTargetsPayload : public AlterationPayload {
    public:

        MultipleAtomTargetsPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        MultipleAtomTargetsPayload(const Payload::Alteration &alteration, const QVector<RPZAtomId> &targetedRPZAtomIds) : AlterationPayload(alteration) {
            this->_setTargetRPZAtomIds(targetedRPZAtomIds);
        }
    
        QVector<RPZAtomId> targetRPZAtomIds() const {
            
            auto list = this->value(QStringLiteral(u"ids")).toList();

            QVector<RPZAtomId> out;
            for(auto &e : list) {
                auto id = e.toULongLong();
                out.append(id);
            }
            
            return out;
        }

        virtual QVariant args() const {
            return this->value(QStringLiteral(u"args"));
        }
    
    private:
        void _setTargetRPZAtomIds(const QVector<RPZAtomId> &targetRPZAtomIds) {
            QVariantList cast;
            for(auto &id : targetRPZAtomIds) {
                cast.append(
                    QString::number(id)
                );
            }
            this->insert(QStringLiteral(u"ids"), cast);
        }
};