#pragma once

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
        RPZMap<RPZAtom> atoms();

    signals:
        void atomsAltered(QVariantHash &payload);

    public slots:
        void alterScene(QVariantHash &payload); 

    protected:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;
        void _emitAlteration(AlterationPayload &payload);

        //atoms list 
        RPZMap<RPZAtom> _atomsById;

        //credentials handling
        QHash<snowflake_uid, QSet<snowflake_uid>> _atomIdsByOwnerId;

        //alter the inner atoms lists
        virtual void _alterSceneGlobal(AlterationPayload &payload);
        virtual RPZAtom* _alterSceneInternal(const AlterationPayload::Alteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration);
};