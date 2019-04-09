#pragma once

#include <QUuid>
#include <QHash>
#include <QSet>
#include <QList>
#include <QVariantList>
#include <QObject>
#include <QDebug>

#include "src/shared/_serializer.hpp"
#include "src/shared/RPZAsset.hpp"

#include "src/shared/AlterationPayload.hpp"

class MapHint : public QObject {

    Q_OBJECT

    public:
        MapHint();
        QList<RPZAsset> fetchHistory();
    
    public slots:
        //network helpers...
        QVariantHash packageForNetworkSend(const RPZAsset::Alteration &state, QList<RPZAsset> &assets);

        //from external App instructions (toolBar, RPZServer...)
        void alterSceneFromAsset(const RPZAsset::Alteration &alteration, RPZAsset &asset);
        void alterSceneFromAssets(const RPZAsset::Alteration &alteration, QList<RPZAsset> &assets);
        void alterSceneFromIds(const RPZAsset::Alteration &alteration, const QList<QUuid> &assetIds); 

    protected:
        bool _preventNetworkAlterationEmission = false;
        void _emitAlteration(const RPZAsset::Alteration &state, QList<RPZAsset> &elements);

        //assets list 
        QHash<QUuid, RPZAsset> _assetsById;

        //credentials handling
        QSet<QUuid> _selfElements;
        QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

        //get assets from list of asset Ids
        QList<RPZAsset> _fetchAssets(const QList<QUuid> &listToFetch) const;

        //alter the inner assets lists
        virtual void _alterSceneGlobal(const RPZAsset::Alteration &alteration, QList<RPZAsset> &assets);
        virtual QUuid _alterSceneInternal(const RPZAsset::Alteration &alteration, RPZAsset &asset);

    signals:
        void assetsAlteredForLocal(const RPZAsset::Alteration &state, QList<RPZAsset> &elements);
        void assetsAlteredForNetwork(const RPZAsset::Alteration &state, QList<RPZAsset> &elements);
};