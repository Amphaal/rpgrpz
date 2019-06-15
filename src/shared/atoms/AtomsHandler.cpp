#include "AtomsHandler.h"

AtomsHandler::AtomsHandler(const AlterationPayload::Source &boundSource) : _source(boundSource) { }

AlterationPayload::Source AtomsHandler::source() {
    return this->_source;
}

void AtomsHandler::handleAlterationRequest(QVariantHash &payload) {
    auto ptr = Payloads::autoCast(payload);
    this->_handlePayload(*ptr);
}

void AtomsHandler::_emitAlteration(AlterationPayload &payload) {

    //define source of payload
    auto source = payload.source();
    if(source == AlterationPayload::Source::Network) return; //prevent resending network payload
    if(source == AlterationPayload::Source::Undefined) payload.changeSource(this->_source); //inner payload, apply own source for send

    emit alterationRequested(payload);
}