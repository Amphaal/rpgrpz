#pragma once

#include <QVariantHash>
#include <QPainterPath>

#include <QString>
#include <QHash>

#include <QFont>
#include <QPen>

#include <QDebug>

#include <QByteArray>
#include <QDataStream>

#include <QGraphicsItem>

#include "src/_libs/snowflake.hpp"
#include "base/Ownable.hpp"
#include "src/helpers/JSONSerializer.hpp"

#include "src/ui/map/graphics/MapViewItemsNotifier.hpp"

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

enum class BrushType { 
    Stamp, 
    Rectangle, 
    Ovale,
    RoundBrush,
    Cutter, 
    Scissors 
};

//order is important for transform handling
enum AtomParameter {
    AssetId,
    AssetName,
    Scale,
    Rotation,
    Text,
    TextSize,
    Layer,
    PenWidth,
    Hidden,
    Locked,
    Shape,
    Position,
    AssetRotation,
    AssetScale,
    BrushStyle
};

class RPZAtom : public Ownable {
    
    public:
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

        QVariant metadata(const AtomParameter &key);
        void unsetMetadata(const AtomParameter &key);
        void setMetadata(const AtomParameter &key, const QVariant &value, bool autoRemove = true);
        void setMetadata(const AtomParameter &key, RPZAtom &base, bool autoRemove = true);

        
        QSet<AtomParameter> editedMetadata();
        QSet<AtomParameter> legalEditedMetadata();
        QSet<AtomParameter> legalParameters();
        QSet<AtomParameter> customizableParams();

        QString assetId();
        QString assetName();
        double scale();
        double rotation();
        double assetScale();
        double assetRotation();
        QString text();
        int textSize();
        int layer();
        QPointF pos();
        int penWidth();
        bool isHidden();
        bool isLocked();
        BrushType brushType();

        QPainterPath shape();
        void setShape(const QPainterPath &path);
        void setShape(const QRectF &rect);

    private:
        static inline const QHash<AtomParameter, QString> _str = {
            { AtomParameter::AssetId, "a_id" },
            { AtomParameter::AssetName, "a_name" },
            { AtomParameter::Scale, "scl" },
            { AtomParameter::Rotation, "deg" },
            { AtomParameter::Text, "txt" },
            { AtomParameter::TextSize, "txt_s" },
            { AtomParameter::Layer, "lyr" },
            { AtomParameter::Position, "pos" },
            { AtomParameter::PenWidth, "pen_w" },
            { AtomParameter::Shape, "shape" },
            { AtomParameter::Hidden, "hid" },
            { AtomParameter::Locked, "lck" },
            { AtomParameter::AssetRotation, "a_deg" },
            { AtomParameter::AssetScale, "a_scl" },
            { AtomParameter::BrushStyle, "brush_t" }
        };

        static inline const QHash<AtomParameter, QVariant> _defaultVal = {
            { AtomParameter::AssetId, "" },
            { AtomParameter::AssetName, "" },
            { AtomParameter::Scale, 1.0 },
            { AtomParameter::Rotation, 0.0 },
            { AtomParameter::Text, "Saisir du texte" },
            { AtomParameter::TextSize, 10 },
            { AtomParameter::Layer, 0 },
            { AtomParameter::Position, NULL },
            { AtomParameter::PenWidth, 1 },
            { AtomParameter::Shape, NULL },
            { AtomParameter::Hidden, false },
            { AtomParameter::Locked, false },
            { AtomParameter::AssetRotation, 0.0 },
            { AtomParameter::AssetScale, 1.0 },
            { AtomParameter::BrushStyle, 0 }
        };

        QString _defaultDescriptor();
        void _setType(const AtomType &type);

        QGraphicsItem* _graphicsItem = nullptr;
};