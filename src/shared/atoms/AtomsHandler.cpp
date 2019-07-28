#include "AtomsHandler.h"

AtomsHandler::AtomsHandler(const AlterationPayload::Source &boundSource) : _source(boundSource) { }

AlterationPayload::Source AtomsHandler::source() {
    return this->_source;
}

void AtomsHandler::handleAlterationRequest(AlterationPayload &payload, bool autoPropagate) {
    auto allowPropagation = this->_handlePayload(payload);
    if(autoPropagate && allowPropagation) this->propagateAlterationPayload(payload);
}

void AtomsHandler::propagateAlterationPayload(AlterationPayload &payload) {

    //if inner payload, apply own source for send
    auto source = payload.source();
    if(source == AlterationPayload::Source::Undefined) payload.changeSource(this->_source); 

    //propagate
    AtomAlterationAcknoledger::propagateAlterationPayload(payload);
    
}