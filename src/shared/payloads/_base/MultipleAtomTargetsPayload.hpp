#pragma once

#include "AlterationPayload.hpp"

#include <QList>

class MultipleAtomTargetsPayload : public AlterationPayload {
    public:   
        QVector<RPZAtom::Id> targetRPZAtomIds() const {
            
            auto list = this->value(QStringLiteral(u"ids")).toList();

            QVector<RPZAtom::Id> out;
            for(auto &e : list) {
                auto id = e.toULongLong();
                out.append(id);
            }
            
            return out;
        }

        virtual QVariant args() const {
            return this->value(QStringLiteral(u"args"));
        }

    protected:
        explicit MultipleAtomTargetsPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        MultipleAtomTargetsPayload(const Payload::Alteration &alteration, const QVector<RPZAtom::Id> &targetedRPZAtomIds) : AlterationPayload(alteration) {
            this->_setTargetRPZAtomIds(targetedRPZAtomIds);
        }
    
    private:
        void _setTargetRPZAtomIds(const QVector<RPZAtom::Id> &targetRPZAtomIds) {
            QVariantList cast;
            for(auto &id : targetRPZAtomIds) {
                cast.append(
                    QString::number(id)
                );
            }
            this->insert(QStringLiteral(u"ids"), cast);
        }
};