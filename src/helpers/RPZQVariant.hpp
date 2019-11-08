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
            GridAlignable = 1100
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

        static bool isAlignableOnGrid(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::GridAlignable).toBool();
        }
        static void setIsAlignableOnGrid(QGraphicsItem* item, bool isAlignable) {
            item->setData((int)RPZQVariant::Roles::GridAlignable, isAlignable);
        }

};
inline uint qHash(const RPZQVariant::Roles &key, uint seed = 0) {return uint(key) ^ seed;}
