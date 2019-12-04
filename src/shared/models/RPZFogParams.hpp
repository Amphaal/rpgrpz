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

        QPainterPath path() const {
            auto rawPath = this->value("p").toByteArray();
            return JSONSerializer::fromByteArray(rawPath);
        }

};