#pragma once

#include <QListWidgetItem>
#include <QGraphicsItem>
#include <QTreeWidgetItem>

#include "src/network/youtube/YoutubeVideoMetadata.h"
#include "src/shared/models/RPZAtom.h"

class RPZQVariant {
    public:
        enum class Roles {
            AtomLayer = 1001,
            AssetHash = 1003,
            AtomVisibility = 1004,
            AtomAvailability = 1005,
            AtomId = 1006,
            YTVideoMetadataPtr = 1007
        };

        static YoutubeVideoMetadata* ytVideoMetadata(QListWidgetItem* item) {
            return item->data((int)RPZQVariant::Roles::YTVideoMetadataPtr).value<YoutubeVideoMetadata*>();
        }
        static void setYTVideoMetadata(QListWidgetItem* item, YoutubeVideoMetadata* metadata) {
            item->setData((int)RPZQVariant::Roles::YTVideoMetadataPtr, QVariant::fromValue<YoutubeVideoMetadata*>(metadata));
        }

        static RPZAtomId atomId(const QTreeWidgetItem* item) {
            return item->data(0, (int)RPZQVariant::Roles::AtomId).toULongLong();
        }
        static void setAtomId(QTreeWidgetItem* item, const RPZAtomId &id) {
            item->setData(0, (int)RPZQVariant::Roles::AtomId, id);
        }
        static RPZAtomId atomId(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AtomId).toULongLong();
        }
        static void setAtomId(QGraphicsItem* item, const RPZAtomId &id) {
            item->setData((int)RPZQVariant::Roles::AtomId, id);
        }

        static int atomLayer(const QTreeWidgetItem* item) {
            return item->data(0, (int)RPZQVariant::Roles::AtomLayer).toInt();
        }
        static void setAtomLayer(QTreeWidgetItem* item, int layer) {
            item->setData(0, (int)RPZQVariant::Roles::AtomLayer, layer);
        }

        static RPZAssetHash assetHash(const QTreeWidgetItem* item) {
            return item->data(0, (int)RPZQVariant::Roles::AssetHash).toString();
        }
        static void setAssetHash(QTreeWidgetItem* item, const RPZAssetHash &hash) {
            item->setData(0, (int)RPZQVariant::Roles::AssetHash, hash);
        }

        static bool atomVisibility(const QModelIndex &index) {
            return index.data((int)RPZQVariant::Roles::AtomVisibility).toBool();
        }
        static void setAtomVisibility(QTreeWidgetItem* item, bool visibility) {
            item->setData(1, (int)RPZQVariant::Roles::AtomVisibility, visibility);
        }

        static bool atomAvailability(const QModelIndex &index) {
            return index.data((int)RPZQVariant::Roles::AtomAvailability).toBool();
        }
        static bool atomAvailability(const QTreeWidgetItem* item) {
            return item->data(1, (int)RPZQVariant::Roles::AtomAvailability).toBool();
        }
        static void setAtomAvailability(QTreeWidgetItem* item, bool availability) {
            item->setData(1, (int)RPZQVariant::Roles::AtomAvailability, availability);
        }
};