#pragma once

#include <QGraphicsView>
#include <QWidget>

#include "MapHint.h"

class MapHintViewBinder : public MapHint {
    
    Q_OBJECT

    public:
        MapHintViewBinder(QGraphicsView* boundGv);

    public slots:
        //on received data, include them into view
        void unpackFromNetworkReceived(const QVariantList &package);

        //add alteration from graphicitem
        void alterSceneFromItems(const MapHint::Alteration &alteration, const QList<QGraphicsItem*> &elements);

        //actions helpers
        QGraphicsPathItem* addDrawing(const QPainterPath &path, const QPen &pen);

    protected:
        QGraphicsView* _boundGv = nullptr;

        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        QUuid _defineId(const MapHint::Alteration &alteration, RPZAsset &asset) override;

        QList<RPZAsset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;
        
        void _alterSceneGlobal(const MapHint::Alteration &alteration, QList<RPZAsset> &assets) override;
        QUuid _alterSceneInternal(const MapHint::Alteration &alteration, RPZAsset &asset) override;

        void _onSceneSelectionChanged();
};