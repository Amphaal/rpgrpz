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

#pragma once

#include <QObject>

#include "src/shared/payloads/Payloads.h"

#include "AlterationHandler.h"

class AlterationHandler;

class AlterationActor {
    public:
        AlterationActor(const Payload::Source &source) : _source(source) {};
        Payload::Source source() const { return this->_source; }

        void payloadTrace(const AlterationPayload &payload) {

            auto payloadSource = payload.source();
            auto payloadType = payload.type();

            qDebug() << "<< [" << payloadSource << ":" << this->_source << "] -" << payloadType;
            
        }

    private:
        Payload::Source _source = Payload::Source::Undefined;
};