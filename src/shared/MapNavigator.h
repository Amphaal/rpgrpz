#pragma once

#include <QGraphicsView>
#include <QWidget>

#include "MapHint.h"

class MapNavigator : public QGraphicsView {
    
    Q_OBJECT

    public:
        MapNavigator(QWidget *parent);

        //on received data, include them into view
        void unpackFromNetworkReceived(const QVariantList &package);

        //from external App instructions (toolBar...)
        void alterScene(const QList<QUuid> &elementIds, const MapHint::Alteration &state);

    //to route from MapHints
    signals:
        void mapElementsAltered(QList<Asset> &elements, const MapHint::Alteration &state);
        void notifyNetwork_mapElementsAltered(QList<Asset> &elements, const MapHint::Alteration &state);

    protected:
        MapHint* _hints = nullptr;

        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;

        QUuid _defineId(const MapHint::Alteration &alteration, Asset &asset);

        QList<Asset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;
        
        void _alterScene(const MapHint::Alteration &alteration, const QList<QGraphicsItem*> &elements);
        void _alterSceneGlobal(const MapHint::Alteration &alteration, QList<Asset> &assets);
        QUuid _alterSceneInternal(const MapHint::Alteration &alteration, Asset &asset);

        void _onSceneSelectionChanged();
};