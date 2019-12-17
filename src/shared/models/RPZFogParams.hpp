#pragma once

#include <QVariantHash>

#include "src/helpers/JSONSerializer.h"

class RPZFogParams : public QVariantHash {

    public:
        enum class Mode {
            PathIsFog,
            PathIsButFog
        };

        RPZFogParams() {};
        explicit RPZFogParams(const QVariantHash &hash) : QVariantHash(hash) {};

        Mode mode() const {
            return (Mode)this->value("m").toInt();
        }

        QList<QPolygonF> polys() const {
            return this->value("polys").value<QList<QPolygonF>>();
        }

};