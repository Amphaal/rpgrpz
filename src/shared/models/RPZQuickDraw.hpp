#pragma once 

#include <QVariantHash>
#include <QPainterPath>

#include "src/helpers/JSONSerializer.h"
#include "src/shared/models/RPZUser.h"

class RPZQuickDraw : public QVariantHash {
    public:
        RPZQuickDraw() {};
        explicit RPZQuickDraw(const QVariantHash &hash) {}
        RPZQuickDraw(const RPZUser::Id &drawerId, const QPainterPath &path) {
            this->insert("drwr", drawerId);
            this->insert("p", JSONSerializer::asBase64(path));
        }

        RPZUser::Id drawer() const {
            return this->value("drwr").toULongLong();
        }

        const QPainterPath path() const {
            return JSONSerializer::toPainterPath(this->value("p").toByteArray());
        }
};