#pragma once

#include <QUuid>
#include <QHash>
#include <QSet>
#include <QList>
#include <QVariantList>
#include <QObject>
#include <QDebug>

#include "_serializer.hpp"
#include "RPZAsset.hpp"

class MapHint : public QObject {

    Q_OBJECT

    public:
        MapHint();
        QList<RPZAsset> fetchHistory();

        //enums
        enum Alteration { Changed, Added, Removed, Selected, Focused, Reset };
        static inline const QList<MapHint::Alteration> networkAlterations = { 
            MapHint::Alteration::Changed, 
            MapHint::Alteration::Added,
            MapHint::Alteration::Removed 
        };
    
    public slots:
        //network helpers...
        QVariantList packageForNetworkSend(const MapHint::Alteration &state, QList<RPZAsset> &assets);

        //from external App instructions (toolBar...)
        void alterSceneFromAsset(const MapHint::Alteration &alteration, RPZAsset &asset);
        void alterSceneFromIds(const MapHint::Alteration &alteration, const QList<QUuid> &assetIds); 

    protected:
        bool _preventNetworkAlterationEmission = false;
        void _emitAlteration(const MapHint::Alteration &state, QList<RPZAsset> &elements);

        //assets list 
        QHash<QUuid, RPZAsset> _assetsById;

        //credentials handling
        QSet<QUuid> _selfElements;
        QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

        //get assets from list of asset Ids
        QList<RPZAsset> _fetchAssets(const QList<QUuid> &listToFetch) const;
        
        //define the id on a receiving / newly Asset
        virtual QUuid _defineId(const Alteration &alteration, RPZAsset &asset);

        //alter the inner assets lists
        virtual void _alterSceneGlobal(const MapHint::Alteration &alteration, QList<RPZAsset> &assets);
        virtual QUuid _alterSceneInternal(const MapHint::Alteration &alteration, RPZAsset &asset);

    signals:
        void assetsAlteredForLocal(const MapHint::Alteration &state, QList<RPZAsset> &elements);
        void assetsAlteredForNetwork(const MapHint::Alteration &state, QList<RPZAsset> &elements);
};