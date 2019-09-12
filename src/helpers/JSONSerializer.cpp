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
        JSONArray[0].toInt(), 
        JSONArray[0].toInt()
    );
}

QPointF JSONSerializer::pointFromDoublePair(const QVariant &doubleList) {
    auto casted = doubleList.toList();
    if(casted.count() != 2) return QPointF();
    return QPointF(
        casted[0].toReal(), 
        casted[1].toReal()
    );
}

QVariant JSONSerializer::pointToDoublePair(const QPointF &point) {
    return QVariantList { point.x(), point.y() };
}

QJsonArray JSONSerializer::pointToDoublePairJSON(const QPointF &point);
    return QJsonArray { point.x(), point.y() };
}

QPointF JSONSerializer::pointFromDoublePair(const QJsonArray &doubleList) {
    if(doubleList.count() != 2) return QPointF();
    return QPointF(
        doubleList[0].toDouble(), 
        doubleList[1].toDouble()
    );
}