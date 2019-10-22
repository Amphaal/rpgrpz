#include "AlterationHandler.h"

AlterationHandler* AlterationHandler::get() {
    if(!_inst) _inst = new AlterationHandler;
    return _inst; 
}

void AlterationHandler::queueAlteration(const AlterationActor* sender, AlterationPayload &payload) {
    return this->_queueAlteration(sender->source(), payload);
}

void AlterationHandler::_queueAlteration(const AlterationPayload::Source &senderSource, AlterationPayload &payload) {
    
    //if initial payload emission, apply sender source for send
    if(payload.source() == AlterationPayload::Source::Undefined && senderSource != AlterationPayload::Source::Undefined) {
        payload.changeSource(senderSource); 
    }

    if(payload.type() == PayloadAlteration::Reset) QMetaObject::invokeMethod(ProgressTracker::get(), "heavyAlterationStarted");

    emit requiresPayloadHandling(payload);
    
}