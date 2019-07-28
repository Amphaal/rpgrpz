#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"
#include "AtomAlterationAcknoledger.hpp"

class AtomsHandler : public QObject, public AtomAlterationAcknoledger {
    
    Q_OBJECT

    public:
        AtomsHandler(const AlterationPayload::Source &boundSource);
        AlterationPayload::Source source();

        void propagateAlterationPayload(AlterationPayload &payload) override;
        void handleAlterationRequest(AlterationPayload &payload) override;

    protected:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;

        virtual void _handlePayload(AlterationPayload &payload) = 0;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) = 0; 
};