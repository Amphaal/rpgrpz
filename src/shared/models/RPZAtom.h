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
            TextSize,
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
        QList<RPZAtom::Parameters> hasMetadata();

        QString assetId();
        QString assetName();
        double scale();
        double rotation();
        QString text();
        int textSize();
        int layer();
        QPointF pos();
        int penWidth();
        bool isHidden();
        bool isLocked();

        QPainterPath shape();
        void setShape(const QPainterPath &path);
        void setShape(const QRectF &rect);

    private:
        static inline const QHash<Parameters, QString> _str = {
            { AssetId, "a_id" },
            { AssetName, "a_name" },
            { Scale, "scl" },
            { Rotation, "deg" },
            { Text, "txt" },
            { TextSize, "txt_s" },
            { Layer, "lyr" },
            { Position, "pos" },
            { PenWidth, "pen_w" },
            { Shape, "shape" },
            { Hidden, "hid" },
            { Locked, "lck" }
        };

        static inline const QHash<Parameters, QVariant> _defaultVal = {
            { AssetId, "" },
            { AssetName, "" },
            { Scale, 1.0 },
            { Rotation, 0.0 },
            { Text, "Saisir du texte" },
            { TextSize, 10 },
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