#include "JSONSerializer.h"

QByteArray JSONSerializer::asBase64(const QPainterPath &path) {
    QByteArray bArray;
    QDataStream stream(&bArray, QIODevice::WriteOnly);
    stream << path;
    return bArray.toBase64();
}

QPainterPath JSONSerializer::fromByteArray(const QByteArray &base64) {
    auto b64 = QByteArray::fromBase64(base64);
    QDataStream stream(&b64, QIODevice::ReadOnly);
    QPainterPath returned;
    stream >> returned;
    return returned;
}

QJsonArray JSONSerializer::fromQSize(const QSize &size) {
    return QJsonArray { size.width(), size.height() };
}

QSize JSONSerializer::toQSize(const QJsonArray &JSONArray) {
    if(JSONArray.count() != 2) return QSize();
    return QSize(
        JSONArray.at(0).toInt(), 
        JSONArray.at(1).toInt()
    );
}

QPointF JSONSerializer::pointFromDoublePair(const QVariant &doubleList) {
    auto casted = doubleList.toList();
    if(casted.count() != 2) return QPointF();
    return QPointF(
        casted.value(0).toReal(), 
        casted.value(1).toReal()
    );
}

QVariant JSONSerializer::pointToDoublePair(const QPointF &point) {
    return QVariantList { point.x(), point.y() };
}

QPointF JSONSerializer::pointFromDoublePair(const QJsonArray &doubleList) {
    if(doubleList.count() != 2) return QPointF();
    return QPointF(
        doubleList.at(0).toDouble(), 
        doubleList.at(1).toDouble()
    );
}

QVariantHash JSONSerializer::serializeUpdates(const AtomUpdates &updates) {
    QVariantHash in;
    for (auto i = updates.constBegin(); i != updates.constEnd(); ++i) {
        
        auto param = i.key();
        auto unserializedVal = i.value();
        
        in.insert(
            QString::number((int)param), 
            JSONSerializer::toSerialized(param, unserializedVal)
        );
    }
    return in;
}

AtomUpdates JSONSerializer::unserializeUpdates(const QVariantHash &serializedUpdates) {
    AtomUpdates out;

    for (auto i = serializedUpdates.begin(); i != serializedUpdates.end(); ++i) {
        auto param = (AtomParameter)i.key().toInt();
        auto serializedVal = i.value();

        out.insert(param, 
            JSONSerializer::fromSerialized(param, serializedVal)
        );
    }

    return out;
}

QVariant JSONSerializer::toSerialized(const AtomParameter &param, const QVariant &unserialized) {
    switch(param) {
        case AtomParameter::ShapeCenter:
        case AtomParameter::Position: {
            return JSONSerializer::pointToDoublePair(unserialized.toPointF());
        }
        break;

        default:
            return unserialized;
    }
}

QVariant JSONSerializer::fromSerialized(const AtomParameter &param, const QVariant &serialized) {
    switch(param) {
        case AtomParameter::ShapeCenter:
        case AtomParameter::Position: {
            return JSONSerializer::pointFromDoublePair(serialized);
        }
        break;

        default:
            return serialized;
    }
}

QJsonArray JSONSerializer::pointToDoublePairJSON(const QPointF &point) {
    return QJsonArray { point.x(), point.y() };
};