#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationActor.hpp"

#include "src/shared/async-ui/progress/ProgressTracker.hpp"

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
        void _queueAlteration(const Payload::Source &senderSource, AlterationPayload &payload);

};