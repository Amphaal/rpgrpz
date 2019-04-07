#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QVariant>

#include <QPainterPath>

// #include <QBrush>
// #include <QPen>

class JSONSerializer {
    
    public:
        static QByteArray toBase64(const QPainterPath &path) {
            QByteArray bArray;
            QDataStream stream(&bArray, QIODevice::WriteOnly);
            stream << path;
            return bArray.toBase64();
        }

        static QPainterPath fromBase64(const QVariant &array) {
            auto b64 = QByteArray::fromBase64(array.toByteArray());
            QDataStream stream(&b64, QIODevice::ReadOnly);
            auto returned = QPainterPath();
            stream >> returned;
            return returned;
        }

};