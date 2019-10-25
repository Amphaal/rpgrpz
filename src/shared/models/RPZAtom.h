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
#include "src/shared/models/_base/Ownable.hpp"
#include "src/helpers/JSONSerializer.h"

#include "src/shared/models/toy/RPZAsset.hpp"

// defined values shared with ToysTreeViewItem type for static casts
enum class RPZAtomType { 
    Undefined, 
    Drawing,
    Text,
    Object, 
    Brush, 
    NPC, 
    Event, 
    PC,
    Background
};
inline uint qHash(const RPZAtomType &key, uint seed = 0) {return uint(key) ^ seed;}

enum class BrushType { 
    Stamp, 
    Rectangle, 
    Ovale,
    RoundBrush,
    Cutter, 
    Scissors 
};
inline uint qHash(const BrushType &key, uint seed = 0) {return uint(key) ^ seed;}


typedef snowflake_uid RPZAtomId;

class RPZAtom : public Serializable {
    
    public:
        RPZAtom();
        explicit RPZAtom(const QVariantHash &hash);
        RPZAtom(RPZAtomId id, const RPZAtomType &type);
        RPZAtom(const RPZAtomType &type);

        RPZAtomType type() const;
        void changeType(const RPZAtomType &type);

        static const QString atomTypeToText(const RPZAtomType &type);
        static const QString toString(const RPZAtomType &type, const QString &assetName);
        const QString toString() const;

        //
        //
        //

        static QVariant getDefaultValueForParam(const AtomParameter &param);

        QVariant metadata(const AtomParameter &key) const;
        void unsetMetadata(const AtomParameter &key);
        void setMetadata(const AtomParameter &key, const QVariant &value);
        void setMetadata(const AtomParameter &key, RPZAtom &base);
        void setMetadata(const AtomUpdates &metadata);

        QSet<AtomParameter> editedMetadata() const;
        AtomUpdates editedMetadataWithValues() const;
        QSet<AtomParameter> legalEditedMetadata() const;
        QSet<AtomParameter> legalParameters() const;
        
        QSet<AtomParameter> customizableParams() const;
        static QSet<AtomParameter> customizableParams(const RPZAtomType &type);

        RPZAssetHash assetHash() const;
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
            { AtomParameter::AssetHash, QStringLiteral(u"a_id") },
            { AtomParameter::AssetName, QStringLiteral(u"a_name") },
            { AtomParameter::Scale, QStringLiteral(u"scl") },
            { AtomParameter::Rotation, QStringLiteral(u"deg") },
            { AtomParameter::Text, QStringLiteral(u"txt") },
            { AtomParameter::TextSize, QStringLiteral(u"txt_s") },
            { AtomParameter::Layer, QStringLiteral(u"lyr") },
            { AtomParameter::Position, QStringLiteral(u"pos") },
            { AtomParameter::PenWidth, QStringLiteral(u"pen_w") },
            { AtomParameter::Shape, QStringLiteral(u"shape") },
            { AtomParameter::Hidden, QStringLiteral(u"hid") },
            { AtomParameter::Locked, QStringLiteral(u"lck") },
            { AtomParameter::AssetRotation, QStringLiteral(u"a_deg") },
            { AtomParameter::AssetScale, QStringLiteral(u"a_scl") },
            { AtomParameter::BrushStyle, QStringLiteral(u"brush_t") },
            { AtomParameter::BrushPenWidth, QStringLiteral(u"brush_w") },
            { AtomParameter::ShapeCenter, QStringLiteral(u"shape_c") }
        };

        static inline const AtomUpdates _defaultVal = {
            { AtomParameter::AssetHash, "" },
            { AtomParameter::AssetName, "" },
            { AtomParameter::Scale, 1.0 },
            { AtomParameter::Rotation, 0.0 },
            { AtomParameter::Text, "" },
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

        void _setType(const RPZAtomType &type);
};

Q_DECLARE_METATYPE(RPZAtom*)
Q_DECLARE_METATYPE(RPZAtom)