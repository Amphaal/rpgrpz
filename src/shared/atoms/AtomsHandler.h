#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"
#include "AtomAlterationAcknoledger.hpp"

#include "src/_libs/asyncfuture.h"
#include <QtConcurrent>

class AtomsHandler : public QObject, public AtomAlterationAcknoledger {
    
    Q_OBJECT

    public:
        AtomsHandler(const AlterationPayload::Source &boundSource, bool autoRegisterAck = true);
        AlterationPayload::Source source();

    protected:
        virtual void _handlePayload(AlterationPayload &payload) = 0;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) = 0; 
        
        QFuture<void> propagateAlterationPayload(AlterationPayload &payload) override;

    private:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;

        QFuture<void> _handleAlterationRequest(AlterationPayload &payload, bool autoPropagate = true) override;
};