#pragma once

#include <QPainterPath>
#include <QByteArray>
#include <QVariant>

class JSONSerializer {
    public:
        static QByteArray asBase64(const QPainterPath &path);
        static QVariant asJSONArray(const QPainterPath &path);

        static QPainterPath fromByteArray(const QByteArray &base64);
        static QPainterPath fromJSONArray(const QVariant &JSONArray);
};