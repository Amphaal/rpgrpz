#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationActor.hpp"

class AlterationActor;

class AlterationHandler : public QObject {
    
    Q_OBJECT

    public:
        static AlterationHandler* get();
        void queueAlteration(const AlterationActor* sender, AlterationPayload &payload);

    signals:
        void requiresPayloadHandling(const AlterationPayload &payload);

    private:
        static inline AlterationHandler* _inst = nullptr;
        void _queueAlteration(const AlterationPayload::Source &senderSource, AlterationPayload &payload);

};