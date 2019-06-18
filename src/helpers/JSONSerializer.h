#pragma once

#include <QPainterPath>
#include <QByteArray>

class JSONSerializer {
    public:
        static QByteArray asBase64(const QPainterPath &path);
        static QPainterPath toPainterPath(const QByteArray &base64);
};