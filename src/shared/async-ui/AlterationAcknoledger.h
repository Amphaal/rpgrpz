#pragma once

#include <functional>

#include <QSet>
#include <QQueue>
#include <QObject>

#include "src/_libs/asyncfuture.h"

#include "src/shared/payloads/Payloads.h"

class AlterationHandler;

class AlterationAcknoledger {

    public:
        AlterationAcknoledger(const AlterationPayload::Source &source, bool senderOnly = false);
        
        AlterationPayload::Source source();

        void queueAlteration(AlterationPayload &payload);
    
    protected:
        virtual QFuture<void> _handleAlterationRequest(AlterationPayload &payload, bool autoPropagate = true) = 0;
        virtual QFuture<void> propagateAlterationPayload(AlterationPayload &payload);

        void _payloadTrace(AlterationPayload &payload);

    private:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;

        void _queueAlteration(AlterationPayload &payload, bool autoPropagate = true);

};