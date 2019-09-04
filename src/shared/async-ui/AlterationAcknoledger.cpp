#include "AlterationAcknoledger.h"

AlterationAcknoledger::AlterationAcknoledger(const AlterationPayload::Source &source) : _source(source) {}

AlterationPayload::Source AlterationAcknoledger::source() const {
    return this->_source;
}

void AlterationAcknoledger::connectToAlterationEmissions() {
    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &AlterationAcknoledger::_ackAlteration
    );
}

void AlterationAcknoledger::_handleAlterationRequest(AlterationPayload &payload) {}

void AlterationAcknoledger::_ackAlteration(const AlterationPayload &payload) {

    //trace
    _payloadTrace(payload);

    //handle
    auto casted = Payloads::autoCast(payload);
    this->_handleAlterationRequest(*casted);

}

void AlterationAcknoledger::payloadTrace(const AlterationPayload::Source &source, const AlterationPayload &payload) {
    auto selfStr = AlterationPayload::SourceAsStr[source];
    auto sourceStr = AlterationPayload::SourceAsStr[payload.source()];
    auto alterationTypeStr = PayloadAlterationAsStr[payload.type()];
    qDebug() << "Alteration :" << selfStr << "received" << alterationTypeStr << "from" << sourceStr;
}

void AlterationAcknoledger::_payloadTrace(const AlterationPayload &payload) {
    return payloadTrace(this->source(), payload);
}