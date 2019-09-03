#include "AlterationAcknoledger.h"

AlterationAcknoledger::AlterationAcknoledger(const AlterationPayload::Source &source, bool autoLinkage) : _source(source) { 
    if(autoLinkage) linkToAlterationHandler();
}

AlterationPayload::Source AlterationAcknoledger::source() const {
    return this->_source;
}

void AlterationAcknoledger::linkToAlterationHandler() {
    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &AlterationAcknoledger::_ackAlteration
    );
}

void AlterationAcknoledger::_handleAlterationRequest(AlterationPayload &payload) {}

void AlterationAcknoledger::_ackAlteration(const AlterationPayload &payload) {

    //trace
    this->_payloadTrace(payload);

    //handle
    auto casted = Payloads::autoCast(payload);
    this->_handleAlterationRequest(*casted);

}


void AlterationAcknoledger::_payloadTrace(const AlterationPayload &payload) {
    auto self = AlterationPayload::SourceAsStr[this->source()];
    auto source = AlterationPayload::SourceAsStr[payload.source()];
    auto alterationType = PayloadAlterationAsString[payload.type()];
    qDebug() << "Alteration :" << self << "received" << alterationType << "from" << source;
}