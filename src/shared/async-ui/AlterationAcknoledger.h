#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationHandler.h"

class AlterationHandler;

class AlterationAcknoledger : public QObject {

    Q_OBJECT

    public:
        AlterationAcknoledger(const AlterationPayload::Source &source);
        AlterationPayload::Source source() const;

        static void payloadTrace(const AlterationPayload::Source &source, const AlterationPayload &payload);

    public slots:
        void connectToAlterationEmissions();

    protected:
        virtual void _handleAlterationRequest(AlterationPayload &payload);

    private slots:
        void _ackAlteration(const AlterationPayload &payload);
        
    private:
        void _payloadTrace(const AlterationPayload &payload);
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;

};