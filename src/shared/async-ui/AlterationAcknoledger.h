#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationHandler.h"

class AlterationHandler;

class AlterationAcknoledger {

    public:
        AlterationAcknoledger(const AlterationPayload::Source &source, bool autoLinkage = true);
        AlterationPayload::Source source();
        void linkToAlterationHandler();

    protected:
        virtual void _handleAlterationRequest(AlterationPayload &payload) = 0;

    private:
        void _ackAlteration(const AlterationPayload &payload);
        void _payloadTrace(const AlterationPayload &payload);
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;

};