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

    private:
        void _setCoordHash(const QHash<QUuid, QPointF> &newCoordsByAtomId) {
            (*this)["coords"] = QVariant::fromValue(newCoordsByAtomId);
        }
};