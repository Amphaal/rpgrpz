#pragma once

#include <QListWidgetItem>
#include <QGraphicsItem>

#include "src/network/youtube/YoutubeVideoMetadata.h"
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
            AllowedToWalk = 7000
        };

        static bool allowedToBeWalked(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AllowedToWalk).toBool();
        }
        static void setAllowedToBeWalked(QGraphicsItem* item, bool allowed) {
            item->setData((int)RPZQVariant::Roles::AllowedToWalk, allowed);
        }

        static YoutubeVideoMetadata* ytVideoMetadata(QListWidgetItem* item) {
            return item->data((int)RPZQVariant::Roles::YTVideoMetadataPtr).value<YoutubeVideoMetadata*>();
        }
        static void setYTVideoMetadata(QListWidgetItem* item, YoutubeVideoMetadata* metadata) {
            item->setData((int)RPZQVariant::Roles::YTVideoMetadataPtr, QVariant::fromValue<YoutubeVideoMetadata*>(metadata));
        }

        static RPZAtom::Id atomId(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AtomId).toULongLong();
        }
        static void setAtomId(QGraphicsItem* item, const RPZAtom::Id &id) {
            item->setData((int)RPZQVariant::Roles::AtomId, id);
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
