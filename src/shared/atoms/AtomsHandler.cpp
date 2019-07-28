#include "AtomsHandler.h"

AtomsHandler::AtomsHandler(const AlterationPayload::Source &boundSource) : _source(boundSource) { }

AlterationPayload::Source AtomsHandler::source() {
    return this->_source;
}

void AtomsHandler::handleAlterationRequest(AlterationPayload &payload) {
    this->_handlePayload(payload);
}

void AtomsHandler::propagateAlteration(AlterationPayload &payload) {

    //if inner payload, apply own source for send
    auto source = payload.source();
    if(source == AlterationPayload::Source::Undefined) payload.changeSource(this->_source); 

    //propagate
    AtomAlterationAcknoledger::propagateAlteration(payload);
    
}