#pragma once

#include <QPainterPath>
#include <QByteArray>
#include <QVariant>
#include <QVariantList>
#include <QSize>
#include <QPointF>

class JSONSerializer {
    public:
        static QByteArray asBase64(const QPainterPath &path);
        static QPainterPath toPainterPath(const QByteArray &base64);

        static QVariant fromQSize(const QSize &size);
        static QSize toQSize(const QVariantList &integerList);
        
        static QVariant fromPointF(const QPointF &point);
        static QPointF toPointF(const QVariantList &doubleList);
};