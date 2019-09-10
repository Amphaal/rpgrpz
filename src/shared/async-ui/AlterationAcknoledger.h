#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationHandler.h"
#include "AlterationActor.hpp"

class AlterationAcknoledger : public QObject, public AlterationActor {

    Q_OBJECT

    public:
        AlterationAcknoledger(const AlterationPayload::Source &source);

    public slots:
        void connectToAlterationEmissions();

    protected:
        virtual void _handleAlterationRequest(AlterationPayload &payload) = 0;

    private slots:
        void _ackAlteration(const AlterationPayload &payload);

};