#include "AtomsHandler.h"

AtomsHandler::AtomsHandler(const AlterationPayload::Source &boundSource) : _source(boundSource) { }

AlterationPayload::Source AtomsHandler::source() {
    return this->_source;
}

void AtomsHandler::handleAlterationRequest(QVariantHash &payload) {
    return this->_handlePayload(AlterationPayload(payload));
}

void AtomsHandler::_emitAlteration(AlterationPayload &payload) {

    //define source of payload
    auto source = payload.source();
    if(source == AlterationPayload::Source::Network) return; //prevent resending network payloay
    if(source == AlterationPayload::Source::Undefined) payload.changeSource(this->_source); //inner payload, apply own source

    emit alterationRequested(payload);
}