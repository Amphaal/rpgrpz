#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QVariant>

#include <QPainterPath>

// #include <QBrush>
// #include <QPen>

class JSONSerializer {
    
    public:

        //
        static QByteArray asBase64(const QPainterPath &path) {
            QByteArray bArray;
            QDataStream stream(&bArray, QIODevice::WriteOnly);
            stream << path;
            return bArray.toBase64();
        }

        static QByteArray asBase64(const QPointF &point) {
            QByteArray bArray;
            QDataStream stream(&bArray, QIODevice::WriteOnly);
            stream << point;
            return bArray.toBase64();
        }


        static QPainterPath toPainterPath(const QByteArray &base64) {
            auto b64 = QByteArray::fromBase64(base64);
            QDataStream stream(&b64, QIODevice::ReadOnly);
            auto returned = QPainterPath();
            stream >> returned;
            return returned;
        }

        static QPointF toPointF(const QByteArray &base64) {
            auto b64 = QByteArray::fromBase64(base64);
            QDataStream stream(&b64, QIODevice::ReadOnly);
            auto returned = QPointF();
            stream >> returned;
            return returned;
        }


};