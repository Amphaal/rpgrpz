#pragma once

#include "base/AlterationPayload.hpp"

#include <QPointF>
#include <QUuid>

class MovedPayload : public AlterationPayload {
    public:
        MovedPayload(const AlterationPayload::Source &source, const QHash<QUuid, QPointF> &newCoordsByAtomId) 
        : AlterationPayload(AlterationPayload::Alteration::Moved, source) { }

        QHash<QUuid, QPointF> coordHash() {
            
            auto base = this->value("coords").toHash();
            
            QHash<QUuid, QPointF> out;
            QVariantHash::iterator i;
            for (i = base.begin(); i != base.end(); ++i) {
                out.insert(QUuid(i.key()), i.value().toPointF());
            }

            return out;
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;
            auto list = this->coordHash();
            QHash<QUuid, QPointF>::iterator i;
            for (i = list.begin(); i != list.end(); ++i) {
                out.insert(i.key().toString(), i.value());
            }
            return out;
        }

    private:
        void _setCoordHash(const QHash<QUuid, QPointF> &newCoordsByAtomId) {
            (*this)["coords"] = QVariant::fromValue(newCoordsByAtomId);
        }
};