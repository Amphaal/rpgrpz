#pragma once

#include "src/helpers/_appContext.h"
#include <QVariantHash>

#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/RPZUser.h"
#include "src/shared/audio/StreamPlayStateTracker.hpp"

class RPZGameSession : public QVariantHash {
    public:
        RPZGameSession() {}
        explicit RPZGameSession(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZGameSession(const RPZUser::Id &selfUserId, const RPZMap<RPZUser> &users, const RPZMap<RPZMessage> &messages, bool isFullSession) {
            this->_setSelfUserId(selfUserId);
            this->_setMessages(messages);
            this->_setUsers(users);
            this->_defineAsFullSession(isFullSession);
        }

        bool isFullSession() const {
            return this->value("fs").toBool();
        }

        const RPZUser::Id selfUserId() const {
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

        void _setUsers(const RPZMap<RPZUser> &users) {
            this->insert("usrs", users.toVMap());
        }

        void _defineAsFullSession(bool isFullSession) {
            this->insert("fs", isFullSession);
        }

        void _setSelfUserId(const RPZUser::Id &selfUserId) {
            this->insert("suid", selfUserId);
        }

};