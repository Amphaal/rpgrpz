#pragma once

#include <QGraphicsView>

#include "MapHint.h"

class MapNavigator : public MapHint, public QGraphicsView {
    
    public:
        MapNavigator();

        void unpackFromNetworkReceived(const QVariantList &package);

        //from external instructions
        void alterScene(const QList<QUuid> &elementIds, const MapHint::Alteration &state) override;

    private:
        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        QList<Asset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;

        void _alterScene(const MapHint::Alteration &alteration, const QList<QGraphicsItem*> &elements);
        void _alterSceneGlobal(const MapHint::Alteration &alteration, QList<Asset> &assets) override;
        QUuid _alterSceneInternal(const MapHint::Alteration &alteration, Asset &asset) override;
}