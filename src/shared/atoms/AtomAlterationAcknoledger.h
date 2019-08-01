#pragma once

#include <functional>

#include <QSet>
#include <QQueue>
#include <QObject>

#include "src/_libs/asyncfuture.h"

#include "src/shared/payloads/Payloads.h"

class AtomAlterationAcknoledger {

    public:
        AtomAlterationAcknoledger(const AlterationPayload::Source &source, bool autoRegisterAck = true);
        ~AtomAlterationAcknoledger();
        
        AlterationPayload::Source source();

        void queueAlteration(AlterationPayload &payload, bool autoPropagate = true);

    protected:
        virtual QFuture<void> _handleAlterationRequest(AlterationPayload &payload, bool autoPropagate = true) = 0;
        virtual QFuture<void> propagateAlterationPayload(AlterationPayload &payload);

        //on reset starting
        virtual void resetAlterationRequested(QFuture<void> &alterationRequest);

        void _payloadTrace(AlterationPayload &payload);


    private:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;

        static inline QSet<AtomAlterationAcknoledger*> _registeredAcknoledgers;
        static inline QQueue<std::function<QFuture<void>()>> _queuedAlterations;
        
        static inline bool _dequeuing = false;
        static QFuture<void> _emptyQueue();

        static void _resetAck(QFuture<void> &resetPromise);
};