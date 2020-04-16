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
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "src/helpers/_appContext.h"
#include <QVariantHash>

#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/RPZUser.h"
#include "src/shared/audio/StreamPlayStateTracker.hpp"
#include "src/shared/models/RPZSharedDocument.hpp"

class RPZGameSession : public QVariantHash {
    public:
        RPZGameSession() {}
        explicit RPZGameSession(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZGameSession(
            const RPZUser::Id &selfUserId, 
            const RPZMap<RPZUser> &users, 
            const RPZMap<RPZMessage> &messages, 
            const RPZSharedDocument::NamesStore &sharedDocuments, 
            bool isFullSession
        ) {
            this->_setSelfUserId(selfUserId);
            this->_setMessages(messages);
            this->_setUsers(users);
            this->_defineAsFullSession(isFullSession);
        }

        bool isFullSession() const {
            return this->value("fs").toBool();
        }

        RPZUser::Id selfUserId() const {
            return this->value("suid").toULongLong();
        }

        const StreamPlayStateTracker streamState() const {
            return StreamPlayStateTracker(this->value("ss").toHash());
        }

        const RPZMap<RPZUser> users() const {
            auto map = this->value("usrs").toMap();
            return RPZMap<RPZUser>::fromVMap(map);
        }

        const RPZMap<RPZMessage> messages() const {
            auto map = this->value("msgs").toMap();
            return RPZMap<RPZMessage>::fromVMap(map);
        }

        const ResetPayload mapPayload() const {
            return ResetPayload(this->value("map").toHash());
        }

        const RPZSharedDocument::NamesStore sharedDocuments() const {
            return this->value("shrd_docs").value<RPZSharedDocument::NamesStore>();
        }

        void setStreamState(const StreamPlayStateTracker &state) {
            this->insert("ss", state);
        } 
        
        void setMapPayload(const ResetPayload &mapPayload) {
            this->insert("map", mapPayload);
        }
    
    private:
        void _setMessages(const RPZMap<RPZMessage> &messages) {
            this->insert("msgs", messages.toVMap());
        }

        void _setSharedDocuments(const RPZSharedDocument::NamesStore &sharedDocuments) {
            this->insert("shrd_docs", QVariant::fromValue(sharedDocuments));
        }

        void _setUsers(const RPZMap<RPZUser> &users) {
            this->insert("usrs", users.toVMap());
        }

        void _defineAsFullSession(bool isFullSession) {
            this->insert("fs", isFullSession);
        }

        void _setSelfUserId(const RPZUser::Id &selfUserId) {
            this->insert("suid", QVariant::fromValue<RPZUser::Id>(selfUserId));
        }

};