#include "AlterationAcknoledger.h"

AlterationAcknoledger::AlterationAcknoledger(const AlterationPayload::Source &source) : AlterationActor(source) {}

void AlterationAcknoledger::connectToAlterationEmissions() {
    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &AlterationAcknoledger::_ackAlteration
    );
}

void AlterationAcknoledger::_ackAlteration(const AlterationPayload &payload) {

    //trace
    this->payloadTrace(payload);

    //handle
    auto casted = Payloads::autoCast(payload);
    this->_handleAlterationRequest(*casted);

}