#pragma once

#include "AtomRelatedPayload.hpp"

#include <QList>

class MultipleAtomTargetsPayload : public AtomRelatedPayload {
    public:   
        QList<RPZAtom::Id> targetRPZAtomIds() const {
            
            auto list = this->value(QStringLiteral(u"ids")).toList();

            QList<RPZAtom::Id> out;
            for(auto &e : list) {
                auto id = e.toULongLong();
                out.append(id);
            }
            
            return out;
        }

        virtual QVariant args() const {
            return this->value(QStringLiteral(u"args"));
        }

        AtomRelatedPayload::RemainingAtomIds restrictTargetedAtoms(const QSet<RPZAtom::Id> &idsToRemove) override {
            
            auto targeted = this->targetRPZAtomIds();

            for(auto &id : targeted) {
                targeted.removeOne(id);
            }

            this->_setTargetRPZAtomIds(targeted);

            return targeted.count();

        };

    protected:
        explicit MultipleAtomTargetsPayload(const QVariantHash &hash) : AtomRelatedPayload(hash) {}
        MultipleAtomTargetsPayload(const Payload::Alteration &alteration, const QList<RPZAtom::Id> &targetedRPZAtomIds) : AtomRelatedPayload(alteration) {
            this->_setTargetRPZAtomIds(targetedRPZAtomIds);
        }
    
    private:
        void _setTargetRPZAtomIds(const QList<RPZAtom::Id> &targetRPZAtomIds) {
            QVariantList cast;
            for(auto &id : targetRPZAtomIds) {
                cast.append(
                    QString::number(id)
                );
            }
            this->insert(QStringLiteral(u"ids"), cast);
        }
};