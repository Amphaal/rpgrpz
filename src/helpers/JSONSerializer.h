#pragma once

#include <QPainterPath>
#include <QByteArray>
#include <QVariant>
#include <QJsonArray>
#include <QSize>
#include <QPointF>

//order is important for transform handling
enum AtomParameter {
    AssetId,
    AssetName,
    BrushStyle,
    BrushPenWidth,
    Scale,
    Rotation,
    Text,
    TextSize,
    Layer,
    PenWidth,
    Hidden,
    Locked,
    Shape,
    Position,
    AssetRotation,
    AssetScale,
    ShapeCenter
};

typedef QHash<AtomParameter, QVariant> AtomUpdates;

class JSONSerializer {
    public:
        static QByteArray asBase64(const QPainterPath &path);
        static QPainterPath fromByteArray(const QByteArray &base64);

        static QJsonArray fromQSize(const QSize &size);
        static QSize toQSize(const QJsonArray &JSONArray); 

        static QPointF pointFromDoublePair(const QJsonArray &doubleList);
        static QPointF pointFromDoublePair(const QVariant &doubleList);
        
        static QVariant pointToDoublePair(const QPointF &point);
        static QJsonArray pointToDoublePairJSON(const QPointF &point);

        static QVariantHash serializeUpdates(const AtomUpdates &updates);
        static AtomUpdates unserializeUpdates(const QVariantHash &serializedUpdates);

        static QVariant toSerialized(const AtomParameter &param, const QVariant &unserialized);
        static QVariant fromSerialized(const AtomParameter &param, const QVariant &serialized);
};