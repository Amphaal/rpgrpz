#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationAcknoledger.h"

class AlterationAcknoledger;

class AlterationHandler : public QObject {
    
    Q_OBJECT

    public:
        static AlterationHandler* get();
        void queueAlteration(const AlterationAcknoledger* sender, AlterationPayload &payload);
        void queueAlteration(const AlterationPayload::Source &senderSource, AlterationPayload &payload);

    signals:
        void requiresPayloadHandling(const AlterationPayload &payload);

    private:
        static inline AlterationHandler* _inst = nullptr;

};