#include "JSONSerializer.h"

QByteArray JSONSerializer::asBase64(const QPainterPath &path) {
    QByteArray bArray;
    QDataStream stream(&bArray, QIODevice::WriteOnly);
    stream << path;
    return bArray.toBase64();
}

QPainterPath JSONSerializer::toPainterPath(const QByteArray &base64) {
    auto b64 = QByteArray::fromBase64(base64);
    QDataStream stream(&b64, QIODevice::ReadOnly);
    auto returned = QPainterPath();
    stream >> returned;
    return returned;
}