#include "AlterationHandler.h"

AlterationHandler* AlterationHandler::get() {
    if(!_inst) _inst = new AlterationHandler;
    return _inst; 
}

void AlterationHandler::queueAlteration(const AlterationActor* sender, AlterationPayload &payload) {
    return this->_queueAlteration(sender->source(), payload);
}

void AlterationHandler::_queueAlteration(const Payload::Source &senderSource, AlterationPayload &payload) {
    
    //if initial payload emission, apply sender source for send
    auto source = payload.source();
    if(source == Payload::Source::Undefined && senderSource != Payload::Source::Undefined) {
        payload.changeSource(senderSource); 
    }

    auto type = payload.type();
    if(type == Payload::Alteration::Reset) {
        QMetaObject::invokeMethod(ProgressTracker::get(), "heavyAlterationStarted");
    }

    emit requiresPayloadHandling(payload);
    
}