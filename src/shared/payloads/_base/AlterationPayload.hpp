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

#pragma once

#include <QVariantHash>

#include "src/_libs/snowflake/snowflake.h"
#include "src/shared/models/RPZAtom.h"

#include "Payload.hpp"

class AlterationPayload : public QVariantHash { 

 public:
        AlterationPayload() {}
        explicit AlterationPayload(const QVariantHash &hash) : QVariantHash(hash) {}
        AlterationPayload(const Payload::Alteration &type) {
            this->_setType(type);
        }

        Payload::Alteration type() const {
            return extractType(*this);
        }

        static Payload::Alteration extractType(const QVariantHash &hash) {
            return (Payload::Alteration)hash.value(QStringLiteral(u"t"), 0).toInt();
        }

        void changeSource(const Payload::Interactor &newSource) {
            this->insert(QStringLiteral(u"s"), (int)newSource);
        }

        Payload::Interactor source() const {
            return (Payload::Interactor)this->value(QStringLiteral(u"s"), 0).toInt();
        }

        bool isNetworkRoutable() const {
            return Payload::networkAlterations.contains(this->type());
        }

        bool undoRedoAllowed() const {
            return Payload::allowsRedoUndo.contains(this->type());
        }

        void tagAsFromTimeline() {
            this->insert(QStringLiteral(u"h"), true);
        }

        bool isFromTimeline() const {
            return this->value(QStringLiteral(u"h")).toBool();
        }

        //necessary for dynamic_cast operations
        virtual ~AlterationPayload() {}

        // friend QDebug operator<<(QDebug debug, const AlterationPayload &c) {
        //     QDebugStateSaver saver(debug);
        //     debug.nospace() << c.type() << ", keys : " << c.keys();
        //     return debug;
        // }

 private:      
        void _setType(const Payload::Alteration &type) {
            this->insert(QStringLiteral(u"t"), (int)type);
        }
};

Q_DECLARE_METATYPE(AlterationPayload)

inline uint qHash(const Payload::Interactor &key, uint seed = 0) {return uint(key) ^ seed;}
