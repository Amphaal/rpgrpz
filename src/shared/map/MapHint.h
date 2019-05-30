#pragma once

#include <QUuid>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QVariantList>
#include <QObject>
#include <QDebug>

#include "src/shared/models/RPZAtom.hpp"
#include "src/shared/models/payloads/base/AlterationPayload.hpp"

class MapHint : public QObject {

    Q_OBJECT

    public:
        MapHint();
        QVector<RPZAtom> atoms();
    
    public slots:
        //network helpers...
        QVariantHash packageForNetworkSend(const AlterationPayload::Alteration &state, QVector<RPZAtom> &atoms);

        //from external App instructions (toolBar, RPZServer...)
        void alterSceneFromAtom(const AlterationPayload::Alteration &alteration, RPZAtom &atom);
        void alterSceneFromAtoms(const AlterationPayload::Alteration &alteration, QVector<RPZAtom> &atoms);
        void alterSceneFromIds(const AlterationPayload::Alteration &alteration, const QVector<QUuid> &atomIds, QVariant &arg); 

    protected:
        bool _preventNetworkAlterationEmission = false;
        void _emitAlteration(const AlterationPayload::Alteration &state, QVector<RPZAtom> &elements);

        //atoms list 
        QHash<QUuid, RPZAtom> _atomsById;

        //credentials handling
        QSet<QUuid> _selfElements;
        QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

        //get atoms from list of atom Ids
        QVector<RPZAtom> _fetchAtoms(const QVector<QUuid> &listToFetch) const;

        //alter the inner atoms lists
        virtual void _alterSceneGlobal(const AlterationPayload::Alteration &alteration, QVector<RPZAtom> &atoms);
        virtual void _alterSceneInternal(const AlterationPayload::Alteration &alteration, RPZAtom &atom);

    signals:
        void atomsAlteredForLocal(const AlterationPayload::Alteration &state, QVector<RPZAtom> &elements);
        void atomsAlteredForNetwork(const AlterationPayload::Alteration &state, QVector<RPZAtom> &elements);
};