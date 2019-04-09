#pragma once

#include <QGraphicsView>
#include <QWidget>

#include "src/shared/AlterationPayload.hpp"

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

    protected:
        QGraphicsView* _boundGv = nullptr;

        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        QList<RPZAsset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;
        
        void _alterSceneGlobal(const RPZAsset::Alteration &alteration, QList<RPZAsset> &assets) override;
        QUuid _alterSceneInternal(const RPZAsset::Alteration &alteration, RPZAsset &asset) override;

        void _onSceneSelectionChanged();
};