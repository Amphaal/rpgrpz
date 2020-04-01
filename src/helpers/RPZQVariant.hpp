#pragma once

#include <QListWidgetItem>
#include <QGraphicsItem>

#include <audiotube/VideoMetadata.h>
#include "src/shared/models/RPZAtom.h"

class RPZQVariant {
    public:
        using AtomTransformationList = QHash<RPZAtom::Parameter, QVariant>;

        enum class Roles {
            AtomId = 1006,
            YTVideoMetadataPtr = 1007,
            GridAlignable = 1100,
            BrushTransform = 2555, 
            IsTemporary = 2666, 
            IsHidden = 2680, 
            BrushDrawStyle = 2767,
            CachedOpacity = 6000,
            AllowedToWalk = 7000,
            AllowedToDefineMoveAbility = 8000,
            CharacterId = 8010,
            CoveredByFog = 9000,
            GraphicsItemToReplace = 10100
        };

        static QGraphicsItem* graphicsItemToReplace(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::GraphicsItemToReplace).value<QGraphicsItem*>();
        }
        static void setGraphicsItemToReplace(QGraphicsItem* item, QGraphicsItem* toReplace) {
            item->setData((int)RPZQVariant::Roles::GraphicsItemToReplace, QVariant::fromValue<QGraphicsItem*>(toReplace));
        }

        static bool allowedToBeWalked(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AllowedToWalk).toBool();
        }
        static void setAllowedToBeWalked(QGraphicsItem* item, bool allowed) {
            item->setData((int)RPZQVariant::Roles::AllowedToWalk, allowed);
        }


        static bool allowedToDefineMoveAbility(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AllowedToDefineMoveAbility).toBool();
        }
        static void setAllowedToDefineMoveAbility(QGraphicsItem* item, bool allowed) {
            item->setData((int)RPZQVariant::Roles::AllowedToDefineMoveAbility, allowed);
        }

        static VideoMetadata* ytVideoMetadata(QListWidgetItem* item) {
            return item->data((int)RPZQVariant::Roles::YTVideoMetadataPtr).value<VideoMetadata*>();
        }
        static void setYTVideoMetadata(QListWidgetItem* item, VideoMetadata* metadata) {
            item->setData((int)RPZQVariant::Roles::YTVideoMetadataPtr, QVariant::fromValue<VideoMetadata*>(metadata));
        }

        static RPZAtom::Id atomId(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AtomId).toULongLong();
        }
        static void setAtomId(QGraphicsItem* item, const RPZAtom::Id &id) {
            item->setData((int)RPZQVariant::Roles::AtomId, QVariant::fromValue<RPZAtom::Id>(id));
        }

        static RPZCharacter::Id boundCharacterId(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::CharacterId).toULongLong();
        }
        static void setBoundCharacterId(QGraphicsItem* item, const RPZCharacter::Id &characterId) {
            item->setData((int)RPZQVariant::Roles::CharacterId, QVariant::fromValue<RPZCharacter::Id>(characterId));
        }

        static bool isGridBound(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::GridAlignable).toBool();
        }
        static void setIsGridBound(QGraphicsItem* item, bool isAlignable) {
            item->setData((int)RPZQVariant::Roles::GridAlignable, isAlignable);
        }

        static auto brushTransform(QGraphicsItem *item) {
            
            auto rawHash = item->data((int)RPZQVariant::Roles::BrushTransform).toHash();
            
            AtomTransformationList out;
            
            for(auto i = rawHash.begin(); i != rawHash.end(); i++) {
                auto param = static_cast<RPZAtom::Parameter>(i.key().toInt());
                out.insert(param, i.value());
            }

            return out;

        }
        static void setBrushTransform(QGraphicsItem *item, const AtomTransformationList &transforms) {

            QVariantHash out;
            for(auto i = transforms.begin(); i != transforms.end(); i++) {
                auto toStr = QString::number((int)i.key());
                out.insert(toStr, i.value());
            }

            item->setData((int)RPZQVariant::Roles::BrushTransform, out);
            
        }

        static bool isTemporary(QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::IsTemporary).toBool();
        }
        static void setIsTemporary(QGraphicsItem* item, bool isTemporary) {
            item->setData((int)RPZQVariant::Roles::IsTemporary, isTemporary);
        }

        static bool isHidden(QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::IsHidden).toBool();
        }
        static void setIsHidden(QGraphicsItem* item, bool isHidden) {
            item->setData((int)RPZQVariant::Roles::IsHidden, isHidden);
        }

        static void setIsCoveredByFog(QGraphicsItem* item, bool isCovered) {
            item->setData((int)RPZQVariant::Roles::CoveredByFog, isCovered);
        }

        static bool isCoveredByFog(QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::CoveredByFog).toBool();
        }

        static double cachedOpacity(QGraphicsItem* item) {
            bool ok = false;
            auto data = item->data((int)RPZQVariant::Roles::CachedOpacity).toDouble(&ok);
            return ok ? data : 1;
        }
        static void setCachedOpacity(QGraphicsItem* item, double cached) {
            item->setData((int)RPZQVariant::Roles::CachedOpacity, cached);
        }


        static RPZAtom::BrushType brushDrawStyle(QGraphicsItem* item) {
            return (RPZAtom::BrushType)item->data((int)RPZQVariant::Roles::BrushDrawStyle).toInt();
        }
        static void setBrushDrawStyle(QGraphicsItem* item, const RPZAtom::BrushType &style) {
            item->setData((int)RPZQVariant::Roles::BrushDrawStyle, (int)style);
        }


};
inline uint qHash(const RPZQVariant::Roles &key, uint seed = 0) {return uint(key) ^ seed;}
