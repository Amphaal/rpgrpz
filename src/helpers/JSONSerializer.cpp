#include "JSONSerializer.h"
#include "src/shared/models/RPZAtom.h"

QByteArray JSONSerializer::asBase64(const QPainterPath &path) {
    QByteArray bArray;
    QDataStream stream(&bArray, QIODevice::WriteOnly);
    stream << path;
    return bArray.toBase64();
}

QPainterPath JSONSerializer::toPainterPath(const QByteArray &base64) {
    auto b64 = QByteArray::fromBase64(base64);
    QDataStream stream(&b64, QIODevice::ReadOnly);
    QPainterPath returned;
    stream >> returned;
    return returned;
}

QVariant JSONSerializer::fromQSize(const QSize &size) {
    return QVariantList { size.width(), size.height() };
}

QSize JSONSerializer::toQSize(const QVariantList &integerList) {
    if(integerList.count() != 2) return QSize();
    return QSize(
        integerList.at(0).toInt(), 
        integerList.at(1).toInt()
    );
}

QPointF JSONSerializer::toPointF(const QVariantList &doubleList) {
    if(doubleList.count() != 2) return QPointF();
    return QPointF(
        doubleList.value(0).toReal(), 
        doubleList.value(1).toReal()
    );
}

QVariant JSONSerializer::fromPointF(const QPointF &point) {
    return QVariantList { point.x(), point.y() };
}

QVariant JSONSerializer::fromPolygons(const QList<QPolygonF> &polys) {
    
    QVariantList in;
    for(auto const &poly : polys) {
        
        QVariantList vPoly;
        for(auto const &point : poly) {
            vPoly.insert(vPoly.size(), JSONSerializer::fromPointF(point));
        }

        in.insert(in.size(), vPoly);

    }

    return in;

}

QList<QPolygonF> JSONSerializer::toPolygons(const QVariantList &rawPolys) {

    QList<QPolygonF> out;
    for(auto const &vPoly : rawPolys) {

        QPolygonF poly;
        for(auto const &vPoint: vPoly.toList()) {
            poly.insert(poly.size(), JSONSerializer::toPointF(vPoint.toList()));
        }

        out.insert(out.size(), poly);

    }

    return out;

}