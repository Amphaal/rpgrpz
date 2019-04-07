#pragma once

#include <QUuid>
#include <QHash>
#include <QSet>
#include <QList>
#include <QVariantList>
#include <QObject>

#include "_serializer.hpp"
#include "Asset.hpp"

class MapHint : public QObject {

    Q_OBJECT

    public:
        MapHint();

        //enums
        enum Alteration { Changed, Added, Removed, Selected, Focused };
        static inline const QList<MapHint::Alteration> networkAlterations = { 
            MapHint::Alteration::Changed, 
            MapHint::Alteration::Added,
            MapHint::Alteration::Removed 
        };

        //network helpers...
        QVariantList packageForNetworkSend(QList<Asset> &assets, const MapHint::Alteration &state);

        //from external App instructions (toolBar...)
        virtual void alterScene(const QList<QUuid> &elementIds, const MapHint::Alteration &state);


    //protected:
        void _emitAlteration(QList<Asset> &elements, const MapHint::Alteration &state);

        //assets list 
        QHash<QUuid, Asset> _assetsById;

        //credentials handling
        QSet<QUuid> _selfElements;
        QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

        //get assets from list of asset Ids
        QList<Asset> _fetchAssets(const QList<QUuid> &listToFetch) const;
        
        //define the id on a receiving / newly Asset
        virtual QUuid _defineId(const Alteration &alteration, Asset &asset);

        //alter the inner assets lists
        void _alterScene(const MapHint::Alteration &alteration, Asset &asset);
        void _alterScene(const MapHint::Alteration &alteration, const QList<QUuid> &elementIds); 
        virtual void _alterSceneGlobal(const MapHint::Alteration &alteration, QList<Asset> &assets);
        virtual QUuid _alterSceneInternal(const MapHint::Alteration &alteration, Asset &asset);

    signals:
        void mapElementsAltered(QList<Asset> &elements, const MapHint::Alteration &state);
        void notifyNetwork_mapElementsAltered(QList<Asset> &elements, const MapHint::Alteration &state);
};