#pragma once

#include <QVariantHash>
#include <QPainterPath>

#include <QByteArray>
#include <QDataStream>

class JSONSerializer {
    public:
        static QByteArray asBase64(const QPainterPath &path) {
            QByteArray bArray;
            QDataStream stream(&bArray, QIODevice::WriteOnly);
            stream << path;
            return bArray.toBase64();
        }

        static QPainterPath toPainterPath(const QByteArray &base64) {
            auto b64 = QByteArray::fromBase64(base64);
            QDataStream stream(&b64, QIODevice::ReadOnly);
            auto returned = QPainterPath();
            stream >> returned;
            return returned;
        }
};

class RPZAtomMetadata : public QVariantHash {
    public:
        RPZAtomMetadata() : QVariantHash() {}
        RPZAtomMetadata(const QVariantHash &hash) : QVariantHash(hash) {}
    
        QString assetId() const {
            return (*this)["a_id"].toString();
        }

        QString assetName() const {
            return (*this)["a_name"].toString();
        }

        QPainterPath shape() const {
            return JSONSerializer::toPainterPath(
                (*this)["shape"].toByteArray()
            );
        }

        int layer() const {
            return (*this)["lyr"].toInt();
        }

        QPointF pos() const {
            return (*this)["pos"].toPointF();
        }

        int penWidth() const {
            return (*this)["pen_w"].toInt();
        }

        void setAssetId(const QString &id) {
            (*this)["a_id"] = id;
        }

        void setAssetName(const QString &name) {
            (*this)["a_name"] = name;
        }

        void setShape(const QPainterPath &path) {
            (*this)["shape"] = JSONSerializer::asBase64(path);
        }

        void setShape(const QRectF &rect) {
            QPainterPath shape;
            shape.addRect(rect);
            this->setShape(shape);
        }

        void setPos(const QPointF &pos) {
            (*this)["pos"] = pos;
        }

        void setPenWidth(int width) {
            (*this)["pen_w"] = width;
        }

        void setLayer(int pos) {
            (*this)["lyr"] = pos;
        }
};