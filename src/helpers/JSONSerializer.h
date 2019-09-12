#pragma once

#include <QPainterPath>
#include <QByteArray>
#include <QVariant>
#include <QJsonArray>
#include <QSize>
#include <QPointF>

class JSONSerializer {
    public:
        static QByteArray asBase64(const QPainterPath &path);
        static QPainterPath fromByteArray(const QByteArray &base64);

        static QJsonArray fromQSize(const QSize &size);
        static QSize toQSize(const QJsonArray &JSONArray); 

        static QPointF pointFromDoublePair(const QJsonArray &doubleList);
        static QPointF pointFromDoublePair(const QVariant &doubleList);
        
        static QVariant pointToDoublePair(const QPointF &point);
        static QJsonArray pointToDoublePairJSON(const QPointF &point) {
            return QJsonArray { point.x(), point.y() };
        };
};