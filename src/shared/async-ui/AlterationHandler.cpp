#include "AlterationHandler.h"

AlterationHandler* AlterationHandler::get() {
    if(!_inst) _inst = new AlterationHandler;
    return _inst; 
}

void AlterationHandler::queueAlteration(const AlterationAcknoledger* sender, AlterationPayload &payload) {
    return this->queueAlteration(sender->source(), payload);
}

void AlterationHandler::queueAlteration(const AlterationPayload::Source &senderSource, AlterationPayload &payload) {
    
    //if initial payload emission, apply sender source for send
    if(payload.source() == AlterationPayload::Source::Undefined && senderSource) {
        payload.changeSource(senderSource); 
    }

    emit requiresPayloadHandling(payload);
    
}