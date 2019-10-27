#pragma once

#include <QListWidgetItem>
#include <QGraphicsItem>

#include "src/network/youtube/YoutubeVideoMetadata.h"
#include "src/shared/models/RPZAtom.h"

class RPZQVariant {
    public:
        enum class Roles {
            AtomId = 1006,
            YTVideoMetadataPtr = 1007
        };

        static YoutubeVideoMetadata* ytVideoMetadata(QListWidgetItem* item) {
            return item->data((int)RPZQVariant::Roles::YTVideoMetadataPtr).value<YoutubeVideoMetadata*>();
        }
        static void setYTVideoMetadata(QListWidgetItem* item, YoutubeVideoMetadata* metadata) {
            item->setData((int)RPZQVariant::Roles::YTVideoMetadataPtr, QVariant::fromValue<YoutubeVideoMetadata*>(metadata));
        }

        static RPZAtomId atomId(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AtomId).toULongLong();
        }
        static void setAtomId(QGraphicsItem* item, const RPZAtomId &id) {
            item->setData((int)RPZQVariant::Roles::AtomId, id);
        }

};
inline uint qHash(const RPZQVariant::Roles &key, uint seed = 0) {return uint(key) ^ seed;}
