#include "AlterationHandler.h"

AlterationHandler* AlterationHandler::get() {
    if(!_inst) _inst = new AlterationHandler;
    return _inst; 
}

void AlterationHandler::queueAlteration(const AlterationAcknoledger* sender, AlterationPayload &payload) {

    //if initial payload emission, apply sender source for send
    auto source = payload.source();
    if(source == AlterationPayload::Source::Undefined && sender) payload.changeSource(sender->source()); 

    emit requiresPayloadHandling(payload);

}
