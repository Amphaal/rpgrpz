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
        MapHint(const AlterationPayload::Source &boundSource);
        AlterationPayload::Source source(); 
        QVector<RPZAtom> atoms();

    signals:
        void atomsAlteredForLocal(const QVariantHash &payload);
        void atomsAlteredForNetwork(const QVariantHash &payload);

    public slots:
        //from external App instructions (toolBar, RPZServer...)
        void alterScene(const QVariantHash &payload); 

    protected:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;
        void _emitAlteration(AlterationPayload &payload);

        //atoms list 
        QHash<QUuid, RPZAtom> _atomsById;

        //credentials handling
        QSet<QUuid> _selfElements;
        QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

        //alter the inner atoms lists
        virtual void _alterSceneGlobal(AlterationPayload &payload);
        virtual RPZAtom* _alterSceneInternal(const AlterationPayload::Alteration &type, QUuid &targetedAtomId, QVariant &atomAlteration);
};