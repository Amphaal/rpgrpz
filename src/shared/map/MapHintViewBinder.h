#pragma once

#include <QGraphicsView>
#include <QWidget>

#include <QFileInfo>

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QPixmap>

#include "src/shared/network/AlterationPayload.hpp"
#include "src/ui/assets/base/AssetsTreeViewModel.hpp"

#include "MapHint.h"

class MapHintViewBinder : public MapHint {
    
    Q_OBJECT

    public:
        MapHintViewBinder(QGraphicsView* boundGv);

    public slots:
        //on received data, include them into view
        void unpackFromNetworkReceived(const QVariantHash &package);

        //add alteration from graphicitem
        void alterSceneFromItems(const RPZAsset::Alteration &alteration, const QList<QGraphicsItem*> &elements);

        //actions helpers
        QGraphicsPathItem* addDrawing(const QPainterPath &path, const QPen &pen);

        QGraphicsItem* temporaryAssetElement(AssetsDatabaseElement* assetElem, AssetsDatabase *database);
        void centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos);
        QGraphicsItem* addAssetElement(QGraphicsItem* item, AssetsDatabaseElement* assetElem, const QPoint &pos);

    protected:
        QGraphicsView* _boundGv = nullptr;

        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        QVector<RPZAsset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;
        
        void _alterSceneGlobal(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) override;
        QUuid _alterSceneInternal(const RPZAsset::Alteration &alteration, RPZAsset &asset) override;

        void _onSceneSelectionChanged();
};