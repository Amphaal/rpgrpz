#pragma once

#include <QMultiHash>

#include <QGraphicsView>
#include <QWidget>

#include <QFileInfo>

#include <QMessageBox>

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QPixmap>

#include "src/shared/network/AlterationPayload.hpp"
#include "src/ui/assets/base/AssetsTreeViewModel.hpp"

#include "src/shared/database/MapDatabase.hpp"

#include "src/ui/map/graphics/MapViewGraphicsScene.hpp"

#include "MapHint.h"

class MapHintViewBinder : public MapHint {
    
    Q_OBJECT

    public:
        MapHintViewBinder(QGraphicsView* boundGv);
        MapViewGraphicsScene* scene();

        //load/unload
        QString stateFilePath();
        bool loadDefaultState();
        bool loadState(QString &filePath);
        bool saveState();
        bool saveStateAs(QString &newFilePath);
        
        bool isDirty();
        void mayWantToSavePendingState();

        bool isRemote();
        bool defineAsRemote(QString &remoteMapDescriptor = QString());

        //replace placeholders
        void replaceMissingAssetPlaceholders(const QString &assetId);

        //on received data, include them into view
        void unpackFromNetworkReceived(const QVariantHash &package);

        //add alteration from graphicitem
        void alterSceneFromItems(const RPZAsset::Alteration &alteration, const QList<QGraphicsItem*> &elements);
        void alterSceneFromItem(const RPZAsset::Alteration &alteration, QGraphicsItem* element);

        //actions helpers
        void addDrawing(const QPainterPath &path, const QPen &pen);

        //D&D assets handling
        void centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos);
        QGraphicsItem* generateTemplateAssetElement(AssetsDatabaseElement* assetElem);
        void addTemplateAssetElement(QGraphicsItem* temporaryItem, AssetsDatabaseElement* assetElem, const QPoint &dropPos);
        
        //
        void handleAnyMovedItems();

        //pen
        QPen getPen() const;
        void setPenColor(QColor &color);
        void setPenSize(int size);

    signals:
        void mapFileStateChanged(const QString &filePath, bool isDirty);
        void requestMissingAsset(const QString &assetIdToRequest);

    private:
        QString _stateFilePath;

        QMultiHash<QString, QGraphicsRectItem*> _missingAssetsIdsFromDb;
        bool _isRemote = false;

        bool _isDirty = false;
        void _setDirty(bool dirty = true);
        void _shouldMakeDirty(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements);

        QGraphicsView* _boundGv = nullptr;

        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        void _unpack(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets);
            QGraphicsItem* _unpack_build(RPZAsset &assetToBuildFrom);
            QGraphicsItem* _unpack_update(const RPZAsset::Alteration &alteration, RPZAsset &assetToUpdateFrom);

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        RPZAsset _fetchAsset(QGraphicsItem* graphicElem) const;
        QVector<RPZAsset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;

        QGraphicsItem* _findBoundGraphicsItem(RPZAsset &asset);

        void _onSceneSelectionChanged();
        void _onSceneItemChanged(QGraphicsItem* item, int alteration);
            QSet<QGraphicsItem*> _itemsWhoNotifiedMovement;

        //drawing
        int _penWidth = 1;
        QColor _penColor = Qt::blue;

        //augmenting MapHint
        void _alterSceneGlobal(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) override;
        QUuid _alterSceneInternal(const RPZAsset::Alteration &alteration, RPZAsset &asset) override;
};