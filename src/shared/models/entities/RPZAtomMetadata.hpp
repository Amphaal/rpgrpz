#pragma once

#include <QVariantHash>
#include <QPainterPath>

#include <QString>
#include <QHash>

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

class RPZAtomMetadata {
    private:
        enum Parameters {
            AssetId,
            AssetName,
            Scale,
            Rotation,
            Text,
            Layer,
            Position,
            PenWidth,
            Shape,
            Hidden,
            Locked
        };

        static inline const QHash<Parameters, QString> _str = {
            { AssetId, "a_id" },
            { AssetName, "a_name" },
            { Scale, "scl" },
            { Rotation, "deg" },
            { Text, "txt" },
            { Layer, "lyr" },
            { Position, "pos" },
            { PenWidth, "pen_w" },
            { Shape, "shape" },
            { Hidden, "hid" },
            { Locked, "lck" }
        };

        QVariantHash* _base = nullptr;

    public:
        RPZAtomMetadata() {}
        RPZAtomMetadata(QVariantHash *base) : _base(base) { }

        QString assetId() const { return this->_base->value(_str[AssetId]).toString(); }
        void setAssetId(const QString &id) { this->_base->insert(_str[AssetId], id); }

        QString assetName() const { return this->_base->value(_str[AssetName]).toString();}
        void setAssetName(const QString &name) { this->_base->insert(_str[AssetName], name); }
        
        double scale() const { return this->_base->value(_str[Scale], 1.0).toDouble();}
        void setScale(const double scale) { this->_base->insert(_str[Scale], scale); }

        double rotation() const { return this->_base->value(_str[Rotation], 0).toDouble(); }
        void setRotation(const double rotation) { this->_base->insert(_str[Rotation], rotation); }

        QString text() const { return this->_base->value(_str[Text], "Saisir du texte").toString(); }
        void setText(const QString &text) { this->_base->insert(_str[Text], text); }

        int layer() const { return this->_base->value(_str[Layer], 0).toInt(); }
        void setLayer(int pos) { this->_base->insert(_str[Layer], pos); }

        QPointF pos() const {
            auto arr = this->_base->value(_str[Position]).toList();
            return arr.isEmpty() ? QPointF() : QPointF(arr[0].toReal(), arr[1].toReal());
        }
        void setPos(const QPointF &pos) {
            QVariantList a { pos.x(), pos.y() };
            this->_base->insert(_str[Position], a);
        }

        int penWidth() const { return this->_base->value(_str[PenWidth], 1).toInt(); }
        void setPenWidth(int width) { this->_base->insert(_str[PenWidth], width); }  

        QPainterPath shape() const {
            return JSONSerializer::toPainterPath(
                this->_base->value(_str[Shape]).toByteArray()
            );
        }
        void setShape(const QPainterPath &path) { this->_base->insert(_str[Shape], JSONSerializer::asBase64(path)); }
        void setShape(const QRectF &rect) {
            QPainterPath shape;
            shape.addRect(rect);
            this->setShape(shape);
        }

        bool isHidden() { return this->_base->value(_str[Hidden]).toBool(); }
        void setHidden(bool isHidden) { this->_base->insert(_str[Hidden], isHidden); }

        bool isLocked() { return this->_base->value(_str[Locked]).toBool(); }
        void setLocked(bool isLocked) { this->_base->insert(_str[Locked], isLocked); }
};