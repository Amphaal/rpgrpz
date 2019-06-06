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

        QString assetId() const { return this->value("a_id").toString(); }
        void setAssetId(const QString &id) { (*this)["a_id"] = id; }

        QString assetName() const { return this->value("a_name").toString();}
        void setAssetName(const QString &name) { (*this)["a_name"] = name; }

        
        double scale() const { return this->value("scl", 1.0).toDouble();}
        void setScale(const double scale) { (*this)["scl"] = scale; }

        double rotation() const { return this->value("deg", 0).toDouble(); }
        void setRotation(const double rotation) { (*this)["deg"] = rotation; }

        QString text() const { return this->value("txt", "Saisir du texte").toString(); }
        void setText(const QString &text) { (*this)["txt"] = text; }

        int layer() const { return this->value("lyr", 0).toInt(); }
        void setLayer(int pos) { (*this)["lyr"] = pos; }

        QPointF pos() const {
            auto arr = this->value("pos").toList();
            return arr.isEmpty() ? QPointF() : QPointF(arr[0].toReal(), arr[1].toReal());
        }
        void setPos(const QPointF &pos) {
            QVariantList a { pos.x(), pos.y() };
            (*this)["pos"] = a;
        }

        int penWidth() const { return this->value("pen_w", 1).toInt(); }
        void setPenWidth(int width) { (*this)["pen_w"] = width; }  

        QPainterPath shape() const {
            return JSONSerializer::toPainterPath(
                this->value("shape").toByteArray()
            );
        }
        void setShape(const QPainterPath &path) {  (*this)["shape"] = JSONSerializer::asBase64(path); }
        void setShape(const QRectF &rect) {
            QPainterPath shape;
            shape.addRect(rect);
            this->setShape(shape);
        }

        bool isHidden() { return this->value("hid").toBool(); }
        void setHidden(bool isHidden) { (*this)["hid"] = isHidden; }

        bool isLocked() { return this->value("lck").toBool(); }
        void setLocked(bool isLocked) { (*this)["lck"] = isLocked; }
};