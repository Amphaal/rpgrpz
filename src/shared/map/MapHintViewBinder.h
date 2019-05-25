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

#include "MapHint.h"

class MapHintViewBinder : public MapHint {
    
    Q_OBJECT

    public:
        MapHintViewBinder(QGraphicsView* boundGv);

        //load/unload
        bool loadState(QString &filePath);
        bool loadDefaultState();
        bool saveState();
        bool saveStateAs(QString &newFilePath);
        QString stateFilePath();
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

        //actions helpers
        void addDrawing(const QPainterPath &path, const QPen &pen);

        //D&D assets handling
        void centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos);
        QGraphicsItem* generateTemplateAssetElement(AssetsDatabaseElement* assetElem);
        void addTemplateAssetElement(QGraphicsItem* temporaryItem, AssetsDatabaseElement* assetElem, const QPoint &dropPos);

    signals:
        void mapFileStateChanged(const QString &filePath, bool isDirty);
        void requestMissingAsset(const QString &assetIdToRequest);

    protected:
        QString _stateFilePath;

        QMultiHash<QString, QGraphicsRectItem*> *_missingAssetsIdsFromDb = nullptr;
        bool _isRemote = false;

        bool _isDirty = false;
        void _setDirty(bool dirty = true);
        void _shouldMakeDirty(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements);

        QGraphicsView* _boundGv = nullptr;

        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        void _unpack(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets);
        QGraphicsPathItem* _addDrawing(const QPainterPath &path, const QPen &pen);
        QGraphicsRectItem* _addMissingAssetPH(QRectF &rect);
        QGraphicsItem* _addGenericImageBasedAsset(const QString &pathToImageFile, qreal opacity = 1, const QPointF &initialPos = QPointF());

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        QVector<RPZAsset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;
        
        void _alterSceneGlobal(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) override;
        QUuid _alterSceneInternal(const RPZAsset::Alteration &alteration, RPZAsset &asset) override;

        void _onSceneSelectionChanged();
};