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
            return (Mode)this->value("fm").toInt();
        }

        void setMode(const RPZFogParams::Mode &mode) {
            this->insert("fm", (int)mode);
        }

        QList<QPolygonF> polys() const {
            //TODO fix
            return this->value("polys").value<QList<QPolygonF>>();
        }

        void setPolys(const QList<QPolygonF> &polys) {
            //TODO fix
            this->insert("polys", QVariant::fromValue(polys));
        }

};