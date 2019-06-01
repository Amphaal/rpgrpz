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
#include "src/ui/assets/base/AssetsTreeViewModel.h"

#include "src/shared/database/MapDatabase.hpp"

#include "src/ui/map/graphics/MapViewGraphicsScene.hpp"

#include "MapHint.h"

class MapHintViewBinder : public MapHint {
    
    Q_OBJECT

    public:
        MapHintViewBinder(QGraphicsView* boundGv);
        MapViewGraphicsScene* scene();
        bool isInTextInteractiveMode();

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

        //actions helpers
        void addDrawing(const QPointF &startPos, const QPainterPath &path, const QPen &pen);

        QGraphicsTextItem* generateGhostTextItem();
        void turnGhostTextIntoDefinitive(QGraphicsTextItem* temporaryText, const QPoint &eventPos);

        //D&D assets handling
        void centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos);
        QGraphicsItem* generateGhostItem(AssetsDatabaseElement* assetElem);
        void turnGhostItemIntoDefinitive(QGraphicsItem* temporaryItem, AssetsDatabaseElement* assetElem, const QPoint &eventPos);
        
        void deleteCurrentSelectionItems();

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
        void _shouldMakeDirty(AlterationPayload &payload);

        QGraphicsView* _boundGv = nullptr;

        QGraphicsItem* _buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom);

        void _crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi);
        RPZAtom* _fetchAtom(QGraphicsItem* graphicElem) const;
        QVector<RPZAtom*> _fetchAtoms(const QList<QGraphicsItem*> &listToFetch) const;

        bool _preventInnerGIEventsHandling = false;
        void _onSceneSelectionChanged();
        void _onSceneItemChanged(QGraphicsItem* item, int changeFlag);
            QSet<QGraphicsItem*> _itemsWhoNotifiedMovement;

        //drawing
        int _penWidth = 1;
        QColor _penColor = Qt::blue;

        //layer
        int _defaultLayer = 0;

        //text interactive
        bool _isInTextInteractiveMode = false;

        //augmenting MapHint
        virtual void _alterSceneGlobal(AlterationPayload &payload) override;
        virtual RPZAtom* _alterSceneInternal(const AlterationPayload::Alteration &type, QUuid &targetedAtomId, QVariant &atomAlteration) override;
};