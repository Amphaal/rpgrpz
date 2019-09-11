#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationHandler.h"

class AlterationHandler;

class AlterationActor {
    public:
        AlterationActor(const AlterationPayload::Source &source) : _source(source) {};
        AlterationPayload::Source source() const { return this->_source; }

        void payloadTrace(const AlterationPayload &payload) {
            // auto selfStr = AlterationPayload::SourceAsStr[this->_source];
            // auto sourceStr = AlterationPayload::SourceAsStr[payload.source()];
            // auto alterationTypeStr = PayloadAlterationAsStr[payload.type()];
            // qDebug() << "Alteration :" << selfStr << "received" << alterationTypeStr << "from" << sourceStr;
        }

    private:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;
};