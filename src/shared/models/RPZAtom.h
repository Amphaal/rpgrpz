#pragma once

#include <QVariantHash>
#include <QPainterPath>

#include <QString>
#include <QHash>

#include <QDebug>

#include <QByteArray>
#include <QDataStream>

#include <QGraphicsItem>

#include "libs/snowflake.hpp"
#include "base/Ownable.hpp"

class JSONSerializer {
    public:
        static QByteArray asBase64(const QPainterPath &path);
        static QPainterPath toPainterPath(const QByteArray &base64);
};

// defined values shared with AssetsDatabaseElement type for static casts
enum class AtomType { 
    Undefined, 
    Drawing,
    Text,
    Object, 
    Brush, 
    NPC, 
    Event, 
    PC 
};

class RPZAtom : public Ownable {
    
    public:
        enum Parameters {
            Type,
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

        RPZAtom();
        RPZAtom(const QVariantHash &hash);
        RPZAtom(const snowflake_uid &id, const AtomType &type, const RPZUser &owner);
        RPZAtom(const AtomType &type);

        QGraphicsItem* graphicsItem();
        void setGraphicsItem(QGraphicsItem* item);

        AtomType type();
        void changeType(const AtomType &type);
        QString descriptor();

        //
        //
        //

        void setMetadata(const Parameters &key, const QVariant &value);
        QVariant metadata(const Parameters &key);

        QString assetId();
        void setAssetId(const QString &id);

        QString assetName();
        void setAssetName(const QString &name);
        
        double scale();
        void setScale(const double scale);

        double rotation();
        void setRotation(const double rotation);

        QString text();
        void setText(const QString &text);

        int layer();
        void setLayer(int pos);

        QPointF pos();
        void setPos(const QPointF &pos);

        int penWidth();
        void setPenWidth(int width); 

        QPainterPath shape();
        void setShape(const QPainterPath &path);
        void setShape(const QRectF &rect);

        bool isHidden();
        void setHidden(bool isHidden);

        bool isLocked();
        void setLocked(bool isLocked);

    private:
        static inline const QHash<Parameters, QString> _str = {
            { Type, "t" },
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

        static inline const QHash<Parameters, QVariant> _defaultVal = {
            { Type, 0 },
            { AssetId, "" },
            { AssetName, "" },
            { Scale, 1.0 },
            { Rotation, 0.0 },
            { Text, "Saisir du texte" },
            { Layer, 0 },
            { Position, NULL },
            { PenWidth, 1 },
            { Shape, NULL },
            { Hidden, false },
            { Locked, false }
        };

        QString _defaultDescriptor();
        void _setType(const AtomType &type);

        QGraphicsItem* _graphicsItem = nullptr;
};