#pragma once

#include <QMultiHash>

#include <QGraphicsView>
#include <QWidget>

#include <QFileInfo>

#include <QMessageBox>

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QPixmap>

#include "src/shared/models/Payloads.h"
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
        void alterSceneFromItems(const AlterationPayload::Alteration &alteration, const QList<QGraphicsItem*> &elements);
        void alterSceneFromItem(const AlterationPayload::Alteration &alteration, QGraphicsItem* element);

        //actions helpers
        void addDrawing(const QPainterPath &path, const QPen &pen);

        //D&D assets handling
        void centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos);
        QGraphicsItem* generateGhostItem(AssetsDatabaseElement* assetElem);
        void turnGhostItemIntoDefinitive(QGraphicsItem* temporaryItem, AssetsDatabaseElement* assetElem, const QPoint &dropPos);
        
        //
        void handleAnyMovedItems();

        //pen
        QPen getPen() const;
        void setPenColor(QColor &color);
        void setPenSize(int size);

        //layer
        void setDefaultLayer(int layer);

    signals:
        void mapFileStateChanged(const QString &filePath, bool isDirty);
        void requestMissingAsset(const QString &assetIdToRequest);

    private:
        QString _stateFilePath;

        QMultiHash<QString, QGraphicsRectItem*> _missingAssetsIdsFromDb;
        bool _isRemote = false;

        bool _isDirty = false;
        void _setDirty(bool dirty = true);
        void _shouldMakeDirty(const AlterationPayload::Alteration &state, QVector<RPZAtom> &elements);

        QGraphicsView* _boundGv = nullptr;

        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;


        QGraphicsItem* _buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom);

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        RPZAtom _fetchAtom(QGraphicsItem* graphicElem) const;
        QVector<RPZAtom> _fetchAtoms(const QList<QGraphicsItem*> &listToFetch) const;

        QGraphicsItem* _findBoundGraphicsItem(const AlterationPayload::Alteration &alteration, RPZAtom &atom);

        void _onSceneSelectionChanged();
        void _onSceneItemChanged(QGraphicsItem* item, int alteration);
            QSet<QGraphicsItem*> _itemsWhoNotifiedMovement;

        //drawing
        int _penWidth = 1;
        QColor _penColor = Qt::blue;

        //layer
        int _defaultLayer = 0;

        //augmenting MapHint
        void _alterSceneGlobal(const AlterationPayload::Alteration &alteration, QVector<RPZAtom> &atoms) override;
        void _alterSceneInternal(const AlterationPayload::Alteration &alteration, RPZAtom &atom) override;
};