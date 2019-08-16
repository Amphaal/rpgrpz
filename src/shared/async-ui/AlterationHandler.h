#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationAcknoledger.h"

class AlterationAcknoledger;

class AlterationHandler : public QObject {
    
    Q_OBJECT

    public:
        static AlterationHandler* get();
        void queueAlteration(AlterationAcknoledger* sender, AlterationPayload &payload);

    signals:
        void requiresPayloadHandling(const AlterationPayload &payload);

    private:
        static inline AlterationHandler* _inst = nullptr;

};