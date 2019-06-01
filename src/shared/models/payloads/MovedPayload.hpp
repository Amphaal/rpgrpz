#pragma once

#include "base/AlterationPayload.hpp"

#include <QPointF>
#include <QUuid>

class MovedPayload : public AlterationPayload {
    public:
        MovedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        MovedPayload(const QHash<QUuid, QPointF> &newCoordsByAtomId) : AlterationPayload(AlterationPayload::Alteration::Moved) {
            this->_setCoordHash(newCoordsByAtomId);
        }

        QHash<QUuid, QPointF> coordHash() {
            
            auto base = this->value("coords").toHash();
            
            QHash<QUuid, QPointF> out;
            for (QVariantHash::iterator i = base.begin(); i != base.end(); ++i) {
                
                //from list of numeric values
                auto arr = i.value().toList();
                auto coord = arr.isEmpty() ? QPointF() : QPointF(arr[0].toReal(), arr[1].toReal());

                //append
                out.insert(QUuid(i.key()), coord);
            
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
                
                //into an array
                auto coord = i.value();
                QVariantList coordsList { coord.x(), coord.y() };

                //append
                cast.insert(i.key().toString(), coordsList);

            }

            (*this)["coords"] = cast;
        }
};