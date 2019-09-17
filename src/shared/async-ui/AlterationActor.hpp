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

            auto payloadSource = payload.source();
            auto payloadType = payload.type();

            auto selfStr = AlterationPayload::SourceAsStr[this->_source];
            auto sourceStr = AlterationPayload::SourceAsStr[payloadSource];
            auto alterationTypeStr = PayloadAlterationAsStr[payloadType];
            qDebug() << "Alteration :" << selfStr << "received" << alterationTypeStr << "from" << sourceStr;
        }

    private:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;
};