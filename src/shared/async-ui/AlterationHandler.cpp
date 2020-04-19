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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "AlterationHandler.h"

AlterationHandler* AlterationHandler::get() {
    if (!_inst) _inst = new AlterationHandler;
    return _inst;
}

void AlterationHandler::queueAlteration(const AlterationInteractor* sender, AlterationPayload &payload) {
    return this->_queueAlteration(sender->interactorId(), payload);
}

void AlterationHandler::_queueAlteration(const Payload::Interactor &senderSource, AlterationPayload &payload) {
    // if initial payload emission, apply sender source for send
    if (payload.source() == Payload::Interactor::Undefined && senderSource != Payload::Interactor::Undefined) {
        payload.changeSource(senderSource);
    }

    auto type = payload.type();
    qDebug() << ">> [" << payload.source() << "] -" << type;

    if (type == Payload::Alteration::Reset) {
        QMetaObject::invokeMethod(ProgressTracker::get(), "heavyAlterationStarted");
    }

    emit requiresPayloadHandling(payload);
}
