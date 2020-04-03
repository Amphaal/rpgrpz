// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

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