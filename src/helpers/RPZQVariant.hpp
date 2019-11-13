#pragma once

#include <QListWidgetItem>
#include <QGraphicsItem>

#include "src/network/youtube/YoutubeVideoMetadata.h"
#include "src/shared/models/RPZAtom.h"

class RPZQVariant {
    public:
        enum class Roles {
            AtomId = 1006,
            YTVideoMetadataPtr = 1007,
            GridAlignable = 1100,
            BrushTransform = 2555, 
            IsTemporary = 2666, 
            BrushDrawStyle = 2767
        };

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

        static QVariantHash brushTransform(QGraphicsItem *item) {
            return item->data((int)RPZQVariant::Roles::BrushTransform).toHash();
        }
        static void setBrushTransform(QGraphicsItem *item, const QVariantHash &transforms) {
            item->setData((int)RPZQVariant::Roles::BrushTransform, transforms);
        }

        static bool isTemporary(QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::IsTemporary).toBool();
        }
        static void setIsTemporary(QGraphicsItem* item, bool isTemporary) {
            item->setData((int)RPZQVariant::Roles::IsTemporary, isTemporary);
        }

        static RPZAtom::BrushType brushDrawStyle(QGraphicsItem* item) {
            return (RPZAtom::BrushType)item->data((int)RPZQVariant::Roles::BrushDrawStyle).toInt();
        }
        static void setBrushDrawStyle(QGraphicsItem* item, const RPZAtom::BrushType &style) {
            item->setData((int)RPZQVariant::Roles::BrushDrawStyle, (int)style);
        }


};
inline uint qHash(const RPZQVariant::Roles &key, uint seed = 0) {return uint(key) ^ seed;}
