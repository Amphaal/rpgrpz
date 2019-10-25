#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationHandler.h"

class AlterationHandler;

class AlterationActor {
    public:
        AlterationActor(const Payload::Source &source) : _source(source) {};
        Payload::Source source() const { return this->_source; }

        void payloadTrace(const AlterationPayload &payload) {

            auto payloadSource = payload.source();
            auto payloadType = payload.type();

            qDebug() << "Alteration :" << this->_source << "|<<" << payloadType << "[" << payloadSource << "]";
            
        }

    private:
        Payload::Source _source = Payload::Source::Undefined;
};