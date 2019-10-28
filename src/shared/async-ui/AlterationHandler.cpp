#include "AlterationHandler.h"

AlterationHandler* AlterationHandler::get() {
    if(!_inst) _inst = new AlterationHandler;
    return _inst; 
}

void AlterationHandler::queueAlteration(const AlterationActor* sender, AlterationPayload &payload) {
    return this->queueAlteration(sender->source(), payload);
}

void AlterationHandler::queueAlteration(const Payload::Source &senderSource, AlterationPayload &payload) {
    
    //if initial payload emission, apply sender source for send
    if(payload.source() == Payload::Source::Undefined && senderSource != Payload::Source::Undefined) {
        payload.changeSource(senderSource); 
    }

    auto type = payload.type();
    qDebug() << payload.source() << type;

    if(type == Payload::Alteration::Reset) {
        QMetaObject::invokeMethod(ProgressTracker::get(), "heavyAlterationStarted");
    }

    emit requiresPayloadHandling(payload);
    
}