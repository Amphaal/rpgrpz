#pragma once

#include <QObject>
#include <QPair>

#include "src/shared/payloads/Payloads.h"
#include "src/shared/async-ui/AlterationAcknoledger.h"

#include "src/_libs/asyncfuture.h"
#include <QtConcurrent>

class AtomsHandler : public AlterationAcknoledger {

    public:
        AtomsHandler(const AlterationPayload::Source &boundSource, bool autoRegisterAck = true);

    protected:
        virtual void _handlePayload(AlterationPayload &payload) = 0;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) = 0; 
        
        QFuture<void> propagateAlterationPayload(AlterationPayload &payload) override;

    private:
        QFuture<void> _handleAlterationRequest(AlterationPayload &payload, bool autoPropagate = true) override;
};