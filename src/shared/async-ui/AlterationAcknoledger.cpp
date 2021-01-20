// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include "AlterationAcknoledger.h"

AlterationAcknoledger::AlterationAcknoledger(const Payload::Interactor &source) : AlterationInteractor(source) {}

void AlterationAcknoledger::connectToAlterationEmissions() {
    // force queued connection to respect queue expected behavior
    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &AlterationAcknoledger::_ackAlteration,
        Qt::ConnectionType::QueuedConnection
    );
}

void AlterationAcknoledger::_ackAlteration(const AlterationPayload &payload) {
    // trace
    this->payloadTrace(payload);

    // handle
    auto casted = Payloads::autoCast(payload);
    this->_handleAlterationRequest(*casted);
}
