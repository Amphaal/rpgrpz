#pragma once

#include <QHash>
#include <QSet>
#include <QVector>
#include <QVariantList>
#include <QObject>
#include <QDebug>

#include "src/shared/models/entities/RPZAtom.hpp"
#include "src/shared/models/Payloads.h"

#include "AtomsHandler.h"

class AtomsStorage : public AtomsHandler {

    public:
        AtomsStorage(const AlterationPayload::Source &boundSource);
        RPZMap<RPZAtom> atoms();
        
    protected:
        QVector<snowflake_uid> _latestDuplication;
        int _duplicationCount = 0;
        void _duplicateAtoms(QVector<snowflake_uid> &atomIdList);

        //atoms list 
        RPZMap<RPZAtom> _atomsById;

        //credentials handling
        QHash<snowflake_uid, QSet<snowflake_uid>> _atomIdsByOwnerId;

        //alter the inner atoms lists
        virtual void _handlePayload(AlterationPayload &payload) override;
        virtual RPZAtom* _handlePayloadInternal(const AlterationPayload::Alteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) override;
};