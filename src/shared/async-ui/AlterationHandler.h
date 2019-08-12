#pragma once

#include <functional>

#include <QSet>
#include <QQueue>
#include <QObject>

#include "src/_libs/asyncfuture.h"

#include "src/shared/payloads/Payloads.h"

class AlterationAcknoledger;

class AlterationHandler {
    
    public:
        static void registerAck(AlterationAcknoledger* toRegister);
        static bool isDequeuing();

    private:
        static inline QSet<AlterationAcknoledger*> _registeredAcknoledgers;
        static inline QQueue<std::function<QFuture<void>()>> _queuedAlterations;
        
        static inline bool _dequeuing = false;
        static QFuture<void> _emptyQueue();

};