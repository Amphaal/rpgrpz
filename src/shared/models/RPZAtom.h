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

#include "src/_libs/snowflake/snowflake.h"
#include "base/Ownable.hpp"
#include "src/helpers/JSONSerializer.h"

#include "src/ui/map/graphics/GraphicsItemsChangeNotifier.h"

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
    BrushStyle,
    BrushPenWidth,
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
    ShapeCenter
};

typedef QHash<AtomParameter, QVariant> AtomUpdates;
typedef QString RPZAssetHash; //file hash of the asset
typedef snowflake_uid RPZAtomId;

class RPZAtom : public Ownable {
    
    public:
        RPZAtom();
        RPZAtom(const QVariantHash &hash);
        RPZAtom(RPZAtomId id, const AtomType &type, const RPZUser &owner);
        RPZAtom(const AtomType &type);

        AtomType type() const;
        void changeType(const AtomType &type);
        QString descriptor() const;

        //
        //
        //

        QVariant metadata(const AtomParameter &key) const;
        void unsetMetadata(const AtomParameter &key);
        void setMetadata(const AtomParameter &key, const QVariant &value, bool autoRemove = true);
        void setMetadata(const AtomParameter &key, RPZAtom &base, bool autoRemove = true);
        void setMetadata(const AtomUpdates &metadata, bool autoRemove = true);

        QSet<AtomParameter> editedMetadata() const;
        AtomUpdates editedMetadataWithValues() const;
        QSet<AtomParameter> legalEditedMetadata() const;
        QSet<AtomParameter> legalParameters() const;
        QSet<AtomParameter> customizableParams() const;

        RPZAssetHash assetId() const;
        QString assetName() const;
        double scale() const;
        double rotation() const;
        double assetScale() const;
        double assetRotation() const;
        QString text() const;
        int textSize() const;
        int layer() const;
        QPointF pos() const;
        int penWidth() const;
        bool isHidden() const;
        bool isLocked() const;
        BrushType brushType() const;
        int brushPenWidth() const;
        QPointF shapeCenter() const;

        QPainterPath shape() const;
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
            { AtomParameter::BrushStyle, "brush_t" },
            { AtomParameter::BrushPenWidth, "brush_w" },
            { AtomParameter::ShapeCenter, "shape_c" }
        };

        static inline const AtomUpdates _defaultVal = {
            { AtomParameter::AssetId, "" },
            { AtomParameter::AssetName, "" },
            { AtomParameter::Scale, 1.0 },
            { AtomParameter::Rotation, 0.0 },
            { AtomParameter::Text, "Saisir du texte" },
            { AtomParameter::TextSize, 10 },
            { AtomParameter::Layer, 0 },
            { AtomParameter::Position, QVariant() },
            { AtomParameter::PenWidth, 1 },
            { AtomParameter::Shape, QVariant() },
            { AtomParameter::Hidden, false },
            { AtomParameter::Locked, false },
            { AtomParameter::AssetRotation, 0.0 },
            { AtomParameter::AssetScale, 1.0 },
            { AtomParameter::BrushStyle, 0 },
            { AtomParameter::BrushPenWidth, 1 },
            { AtomParameter::ShapeCenter, QVariant() }
        };

        QString _defaultDescriptor() const;
        void _setType(const AtomType &type);

        QGraphicsItem* _graphicsItem = nullptr;
};