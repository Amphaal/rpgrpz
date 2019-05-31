#pragma once

#include "base/AlterationPayload.hpp"

#include <QPointF>
#include <QUuid>

class MovedPayload : public AlterationPayload {
    public:
        MovedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        MovedPayload(const QHash<QUuid, QPointF> &newCoordsByAtomId) : AlterationPayload(AlterationPayload::Alteration::Moved) {}

        QHash<QUuid, QPointF> coordHash() {
            
            auto base = this->value("coords").toHash();
            
            QHash<QUuid, QPointF> out;
            for (QVariantHash::iterator i = base.begin(); i != base.end(); ++i) {
                out.insert(QUuid(i.key()), i.value().toPointF());
            }

            return out;
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;
            auto list = this->coordHash();
            for (QHash<QUuid, QPointF>::iterator i = list.begin(); i != list.end(); ++i) {
                out.insert(i.key().toString(), i.value());
            }
            return out;
        }

    private:
        void _setCoordHash(const QHash<QUuid, QPointF> &newCoordsByAtomId) {
            QVariantHash cast;
            for (QHash<QUuid, QPointF>::const_iterator i = newCoordsByAtomId.begin(); i != newCoordsByAtomId.end(); ++i) {
                cast.insert(i.key().toString(), i.value());
            }
            (*this)["coords"] = cast;
        }
};