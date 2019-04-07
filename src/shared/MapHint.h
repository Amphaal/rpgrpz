#pragma once

#include <QUuid>
#include <QHash>
#include <QSet>
#include <QList>
#include <QVariantList>


#include "_serializer.hpp"
#include "Asset.hpp"

class MapHint {
    public:
        MapHint();

        enum Alteration { Changed, Added, Removed, Selected, Focused };
        static inline const QList<MapHint::Alteration> networkAlterations = { 
            MapHint::Alteration::Changed, 
            MapHint::Alteration::Added,
            MapHint::Alteration::Removed 
        };

        //network helpers...
        QVariantList packageForNetworkSend(QList<Asset> &assets, const MapHint::Alteration &state);

        virtual void alterScene(const QList<QUuid> &elementIds, const MapHint::Alteration &state);

    signals:
        void mapElementsAltered(QList<Asset> &elements, const MapHint::Alteration &state);
        void notifyNetwork_mapElementsAltered(QList<Asset> &elements, const MapHint::Alteration &state);

    protected:

        void _emitAlteration(QList<Asset> &elements, const MapHint::Alteration &state);

        QHash<QUuid, Asset> _assetsById;
        QSet<QUuid> _selfElements;
        QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

        QList<Asset> _fetchAssets(const QList<QUuid> &listToFetch) const;

        void _alterScene(const MapHint::Alteration &alteration, const QList<QUuid> &elementIds);
        void _alterScene(const MapHint::Alteration &alteration, Asset &asset);
        virtual void _alterSceneGlobal(const MapHint::Alteration &alteration, QList<Asset> &assets);
        virtual QUuid _alterSceneInternal(const MapHint::Alteration &alteration, Asset &asset);
}