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
#include "src/shared/models/character/RPZCharacter.hpp"

class RPZAtom : public Serializable {

    public:
    
        //order is important for transform handling
        enum class Parameter {
            Unknown,
            AssetHash,
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
            ShapeCenter,
            EventShortDescription,
            CharacterId,
            EventDescription,
            DefaultPlayerColor,
            NPCAttitude,
            NPCShortName,
            NPCDescription,
            CharacterName,
            NPCHealth,
            MaxNPCHealth
        };

        enum class Category {
            Unknown,
            Interactive,
            Layout
        };

        enum class Type { 
            Undefined, 
            Drawing,
            Text,
            Object, 
            Brush, 
            NPC, 
            Event, 
            Player,
            Background
        };

        enum class BrushType {
            Unknown, 
            Stamp, 
            Rectangle, 
            Ovale,
            RoundBrush,
            Cutter, 
            Scissors 
        };

        enum class NPCType {
            Unknown,
            Neutral,
            Hostile,
            Friendly
        };

        typedef SnowFlake::Id Id;
        typedef int Layer;
        typedef QHash<RPZAtom::Parameter, QVariant> Updates;
        typedef QHash<RPZAtom::Id, RPZAtom::Updates> ManyUpdates;
        
        static const inline QHash<RPZAtom::Type, QString> iconPathByAtomType = {
            { RPZAtom::Type::Event, QStringLiteral(u":/icons/app/manager/event.png") },
            { RPZAtom::Type::NPC, QStringLiteral(u":/icons/app/manager/npc.png") },
            { RPZAtom::Type::Drawing, QStringLiteral(u":/icons/app/tools/pen.png") },
            { RPZAtom::Type::Text, QStringLiteral(u":/icons/app/tools/text.png") },
            { RPZAtom::Type::Player, QStringLiteral(u":/icons/app/connectivity/cloak.png") },
        };

        static const inline QHash<RPZAtom::Type, RPZAtom::Parameter> descriptorsByAtomType {
            { RPZAtom::Type::Player, RPZAtom::Parameter::CharacterName },
            { RPZAtom::Type::Event, RPZAtom::Parameter::EventShortDescription },
            { RPZAtom::Type::NPC, RPZAtom::Parameter::NPCShortName },
            { RPZAtom::Type::Object, RPZAtom::Parameter::AssetName },
            { RPZAtom::Type::Brush, RPZAtom::Parameter::AssetName }
        };

        static const inline QHash<RPZAtom::Type, QString> atomTypeDescr {
            { RPZAtom::Type::Drawing, QT_TRANSLATE_NOOP("QObject", "Drawing") },
            { RPZAtom::Type::Text, QT_TRANSLATE_NOOP("QObject", "Text") },
            { RPZAtom::Type::Object, QT_TRANSLATE_NOOP("QObject", "Object") },
            { RPZAtom::Type::Brush, QT_TRANSLATE_NOOP("QObject", "Brush") },
            { RPZAtom::Type::Undefined, QT_TRANSLATE_NOOP("QObject", "Atom") },
            { RPZAtom::Type::Event, QT_TRANSLATE_NOOP("QObject", "Event") },
            { RPZAtom::Type::Player, QT_TRANSLATE_NOOP("QObject", "Player") },
        };

        static const inline QHash<RPZAtom::Type, QString> atomCategoryTypeDescr {
            { RPZAtom::Type::NPC, QT_TRANSLATE_NOOP("QObject", "NPCs") },
            { RPZAtom::Type::Event, QT_TRANSLATE_NOOP("QObject", "Events") },
            { RPZAtom::Type::Player, QT_TRANSLATE_NOOP("QObject", "Players") },
        };

        static inline const QList<RPZAtom::Type> assetBasedAtom {
            RPZAtom::Type::Object, 
            RPZAtom::Type::Brush,
            RPZAtom::Type::Background,
            RPZAtom::Type::NPC
        };
        
        //
        //
        //

        static QVariantHash serializeUpdates(const RPZAtom::Updates &updates);
        static RPZAtom::Updates unserializeUpdates(const QVariantHash &serializedUpdates);

        static QVariant toSerialized(const RPZAtom::Parameter &param, const QVariant &unserialized);
        static QVariant fromSerialized(const RPZAtom::Parameter &param, const QVariant &serialized);

        //
        //
        //

        RPZAtom();
        explicit RPZAtom(const QVariantHash &hash);
        RPZAtom(RPZAtom::Id id, const RPZAtom::Type &type);
        RPZAtom(const RPZAtom::Type &type);

        RPZAtom::Type type() const;
        void changeType(const RPZAtom::Type &type);

        static const QString toString(const RPZAtom::Type &type, const QString &description = QString());
        const QString toString() const;

        bool isAssetBased() const;
        RPZAtom::Category category() const;
        static RPZAtom::Category category(const RPZAtom::Type &type);
        bool isRestrictedAtom() const;

        //
        //
        //

        static QVariant getDefaultValueForParam(const RPZAtom::Parameter &param);

        QVariant metadata(const RPZAtom::Parameter &key) const;
        void unsetMetadata(const RPZAtom::Parameter &key);
        void setMetadata(const RPZAtom::Parameter &key, const QVariant &value);
        void setMetadata(const RPZAtom::Parameter &key, RPZAtom &base);
        void setMetadata(const RPZAtom::Updates &metadata);

        QSet<RPZAtom::Parameter> editedMetadata() const;
        RPZAtom::Updates editedMetadataWithValues() const;
        QSet<RPZAtom::Parameter> legalEditedMetadata() const;
        QSet<RPZAtom::Parameter> legalParameters() const;
        
        QSet<RPZAtom::Parameter> customizableParams() const;
        static QSet<RPZAtom::Parameter> customizableParams(const RPZAtom::Type &type);

        RPZAsset::Hash assetHash() const;
        QString assetName() const;
        double scale() const;
        double rotation() const;
        double assetScale() const;
        double assetRotation() const;
        QString text() const;
        int textSize() const;
        RPZAtom::Layer layer() const;
        QPointF pos() const;
        int penWidth() const;
        bool isHidden() const;
        bool isLocked() const;
        RPZAtom::BrushType brushType() const;
        int brushPenWidth() const;
        QPointF shapeCenter() const;
        const QColor defaultPlayerColor() const;
        const RPZCharacter::Id characterId() const;
        const QString characterName() const;
        const QString NPCShortName() const;

        QPainterPath shape() const;
        void setShape(const QPainterPath &path);
        void setShape(const QRectF &rect);

    private:
        static inline const QList<RPZAtom::Type> _layoutAtom {
            RPZAtom::Type::Drawing,
            RPZAtom::Type::Text,
            RPZAtom::Type::Object, 
            RPZAtom::Type::Brush,
            RPZAtom::Type::Background
        };

        static inline const QList<RPZAtom::Type> _restrictedAtom {
            RPZAtom::Type::Event
        };

        static inline const QHash<RPZAtom::Parameter, QString> _str = {
            { RPZAtom::Parameter::AssetHash, QStringLiteral(u"a_id") },
            { RPZAtom::Parameter::AssetName, QStringLiteral(u"a_name") },
            { RPZAtom::Parameter::Scale, QStringLiteral(u"scl") },
            { RPZAtom::Parameter::Rotation, QStringLiteral(u"deg") },
            { RPZAtom::Parameter::Text, QStringLiteral(u"txt") },
            { RPZAtom::Parameter::TextSize, QStringLiteral(u"txt_s") },
            { RPZAtom::Parameter::Layer, QStringLiteral(u"lyr") },
            { RPZAtom::Parameter::Position, QStringLiteral(u"pos") },
            { RPZAtom::Parameter::PenWidth, QStringLiteral(u"pen_w") },
            { RPZAtom::Parameter::Shape, QStringLiteral(u"shape") },
            { RPZAtom::Parameter::Hidden, QStringLiteral(u"hid") },
            { RPZAtom::Parameter::Locked, QStringLiteral(u"lck") },
            { RPZAtom::Parameter::AssetRotation, QStringLiteral(u"a_deg") },
            { RPZAtom::Parameter::AssetScale, QStringLiteral(u"a_scl") },
            { RPZAtom::Parameter::BrushStyle, QStringLiteral(u"brush_t") },
            { RPZAtom::Parameter::BrushPenWidth, QStringLiteral(u"brush_w") },
            { RPZAtom::Parameter::ShapeCenter, QStringLiteral(u"shape_c") },
            { RPZAtom::Parameter::CharacterId, QStringLiteral(u"chr_id") },
            { RPZAtom::Parameter::EventDescription, QStringLiteral(u"descr") },
            { RPZAtom::Parameter::EventShortDescription, QStringLiteral(u"s_descr") },
            { RPZAtom::Parameter::DefaultPlayerColor, QStringLiteral(u"color") },
            { RPZAtom::Parameter::NPCAttitude, QStringLiteral(u"npc_t") },
            { RPZAtom::Parameter::NPCShortName, QStringLiteral(u"npc_sn") },
            { RPZAtom::Parameter::NPCDescription, QStringLiteral(u"npc_descr") },
            { RPZAtom::Parameter::CharacterName, QStringLiteral(u"char_nm") },
            { RPZAtom::Parameter::NPCHealth, QStringLiteral(u"npc_h") },
            { RPZAtom::Parameter::MaxNPCHealth, QStringLiteral(u"npc_maxh") }
        };

        static inline const RPZAtom::Updates _defaultVal = {
            { RPZAtom::Parameter::AssetHash, "" },
            { RPZAtom::Parameter::AssetName, "" },
            { RPZAtom::Parameter::Scale, 1.0 },
            { RPZAtom::Parameter::Rotation, 0.0 },
            { RPZAtom::Parameter::Text, "" },
            { RPZAtom::Parameter::TextSize, 10 },
            { RPZAtom::Parameter::Layer, 0 },
            { RPZAtom::Parameter::Position, QVariant() },
            { RPZAtom::Parameter::PenWidth, 1 },
            { RPZAtom::Parameter::Shape, QVariant() },
            { RPZAtom::Parameter::Hidden, false },
            { RPZAtom::Parameter::Locked, false },
            { RPZAtom::Parameter::AssetRotation, 0.0 },
            { RPZAtom::Parameter::AssetScale, 1.0 },
            { RPZAtom::Parameter::BrushStyle, (int)RPZAtom::BrushType::Stamp },
            { RPZAtom::Parameter::BrushPenWidth, 10 },
            { RPZAtom::Parameter::ShapeCenter, QVariant() },
            { RPZAtom::Parameter::CharacterId, 0 },
            { RPZAtom::Parameter::EventDescription, "" },
            { RPZAtom::Parameter::EventShortDescription, "" },
            { RPZAtom::Parameter::DefaultPlayerColor, QColor() },
            { RPZAtom::Parameter::NPCAttitude, 0 },
            { RPZAtom::Parameter::NPCShortName, "" },
            { RPZAtom::Parameter::NPCDescription, "" },
            { RPZAtom::Parameter::CharacterName, "" },
            { RPZAtom::Parameter::NPCHealth, -1 },
            { RPZAtom::Parameter::MaxNPCHealth, -1 }
        };

        void _setType(const RPZAtom::Type &type);

        static const QString _atomTypeToText(const RPZAtom::Type &type);

};
inline uint qHash(const RPZAtom::Category &key, uint seed = 0) {return uint(key) ^ seed;}
inline uint qHash(const RPZAtom::Type &key, uint seed = 0) {return uint(key) ^ seed;}
inline uint qHash(const RPZAtom::BrushType &key, uint seed = 0) {return uint(key) ^ seed;}
inline uint qHash(const RPZAtom::Parameter &key, uint seed = 0) {return uint(key) ^ seed;}
inline uint qHash(const RPZAtom::NPCType &key, uint seed = 0) {return uint(key) ^ seed;}

Q_DECLARE_METATYPE(RPZAtom*)
Q_DECLARE_METATYPE(RPZAtom)