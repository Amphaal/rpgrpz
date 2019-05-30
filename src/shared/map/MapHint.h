#pragma once

#include <QUuid>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QVariantList>
#include <QObject>
#include <QDebug>

#include "src/shared/models/entities/RPZAtom.hpp"
#include "src/shared/models/Payloads.h"

class MapHint : public QObject {

    Q_OBJECT

    public:
        MapHint();
        QVector<RPZAtom> atoms();

    signals:
        void atomsAlteredForLocal(const QVariantHash &payload);
        void atomsAlteredForNetwork(const QVariantHash &payload);

    public slots:
        //network helpers...
        QVariantHash packageForNetworkSend(const AlterationPayload::Alteration &state, QVector<RPZAtom> &atoms);

        //from external App instructions (toolBar, RPZServer...)
        void alterScene(const QVariantHash &payload); 

    protected:
        bool _preventNetworkAlterationEmission = false;
        void _emitAlteration(const AlterationPayload &payload);

        //atoms list 
        QHash<QUuid, RPZAtom> _atomsById;

        //credentials handling
        QSet<QUuid> _selfElements;
        QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

        //get atoms from list of atom Ids
        QVector<RPZAtom> _fetchAtoms(const QVector<QUuid> &listToFetch);

        //alter the inner atoms lists
        virtual void _alterSceneGlobal(const AlterationPayload::Alteration &alteration, QVector<RPZAtom> &atoms);
        virtual void _alterSceneInternal(const AlterationPayload::Alteration &alteration, RPZAtom &atom);


};