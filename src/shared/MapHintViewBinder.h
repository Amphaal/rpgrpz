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

    protected:
        QGraphicsView* _boundGv = nullptr;

        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        QUuid _defineId(const MapHint::Alteration &alteration, Asset &asset) override;

        QList<Asset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;
        
        void _alterSceneGlobal(const MapHint::Alteration &alteration, QList<Asset> &assets) override;
        QUuid _alterSceneInternal(const MapHint::Alteration &alteration, Asset &asset) override;

        void _onSceneSelectionChanged();
};