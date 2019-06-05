#pragma once

#include "base/AlterationPayload.hpp"

#include <QPointF>

class MovedPayload : public AlterationPayload {
    public:
        MovedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        MovedPayload(const QHash<snowflake_uid, QPointF> &newCoordsByAtomId) : AlterationPayload(AlterationPayload::Alteration::Moved) {
            this->_setCoordHash(newCoordsByAtomId);
        }

        QHash<snowflake_uid, QPointF> coordHash() {
            
            auto base = this->value("coords").toHash();
            
            QHash<snowflake_uid, QPointF> out;
            for (QVariantHash::iterator i = base.begin(); i != base.end(); ++i) {
                
                //from list of numeric values
                auto arr = i.value().toList();
                auto coord = arr.isEmpty() ? QPointF() : QPointF(arr[0].toReal(), arr[1].toReal());

                //append
                out.insert(i.key().toULongLong(), coord);
            
            }

            return out;
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;
            auto list = this->coordHash();
            for (QHash<snowflake_uid, QPointF>::iterator i = list.begin(); i != list.end(); ++i) {
                out.insert(QString::number(i.key()), i.value());
            }
            return out;
        }

    private:
        void _setCoordHash(const QHash<snowflake_uid, QPointF> &newCoordsByAtomId) {
            
            QVariantHash cast;
            for (QHash<snowflake_uid, QPointF>::const_iterator i = newCoordsByAtomId.begin(); i != newCoordsByAtomId.end(); ++i) {
                
                //into an array
                auto coord = i.value();
                QVariantList coordsList { coord.x(), coord.y() };

                //append
                cast.insert(QString::number(i.key()), coordsList);

            }

            (*this)["coords"] = cast;
        }
};