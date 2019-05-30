#pragma once

#include <QUuid>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QVariantList>
#include <QObject>
#include <QDebug>

#include "src/shared/_serializer.hpp"
#include "src/shared/network/RPZAtom.hpp"
#include "src/shared/network/AlterationPayload.hpp"

class MapHint : public QObject {

    Q_OBJECT

    public:
        MapHint();
        QVector<RPZAtom> atoms();
    
    public slots:
        //network helpers...
        QVariantHash packageForNetworkSend(const RPZAtom::Alteration &state, QVector<RPZAtom> &atoms);

        //from external App instructions (toolBar, RPZServer...)
        void alterSceneFromAtom(const RPZAtom::Alteration &alteration, RPZAtom &atom);
        void alterSceneFromAtoms(const RPZAtom::Alteration &alteration, QVector<RPZAtom> &atoms);
        void alterSceneFromIds(const RPZAtom::Alteration &alteration, const QVector<QUuid> &atomIds, QVariant &arg); 

    protected:
        bool _preventNetworkAlterationEmission = false;
        void _emitAlteration(const RPZAtom::Alteration &state, QVector<RPZAtom> &elements);

        //atoms list 
        QHash<QUuid, RPZAtom> _atomsById;

        //credentials handling
        QSet<QUuid> _selfElements;
        QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

        //get atoms from list of atom Ids
        QVector<RPZAtom> _fetchAtoms(const QVector<QUuid> &listToFetch) const;

        //alter the inner atoms lists
        virtual void _alterSceneGlobal(const RPZAtom::Alteration &alteration, QVector<RPZAtom> &atoms);
        virtual void _alterSceneInternal(const RPZAtom::Alteration &alteration, RPZAtom &atom);

    signals:
        void atomsAlteredForLocal(const RPZAtom::Alteration &state, QVector<RPZAtom> &elements);
        void atomsAlteredForNetwork(const RPZAtom::Alteration &state, QVector<RPZAtom> &elements);
};