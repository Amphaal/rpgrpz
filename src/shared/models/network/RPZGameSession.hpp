#pragma once

#include "src/helpers/_appContext.h"
#include <QVariantHash>

#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/RPZUser.h"
#include "src/shared/audio/StreamPlayStateTracker.hpp"

class RPZGameSession : public QVariantHash {
    public:
        explicit RPZGameSession(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZGameSession(const RPZUser::Id &selfUserId, const RPZMap<RPZUser> &users, const RPZMap<RPZMessage> &messages, bool isFullSession) {
            this->_setSelfUserId(selfUserId);
            this->_setMessages(messages);
            this->_setUsers(users);
            this->_defineAsFullSession(isFullSession);
        }

        bool isFullSession() const {
            this->value("fs").toBool();
        }

        const RPZUser::Id selfUserId() const {
            return this->value("suid").toULongLong();
        }

        const StreamPlayStateTracker streamState() const {
            return this->value("ss").value<StreamPlayStateTracker>();
        }

        const RPZMap<RPZUser> users() const {
            return this->value("usrs").value<RPZMap<RPZUser>>();
        }

        const QVector<RPZMessage> messages() const {
            return this->value("msgs").value<QVector<RPZMessage>>();
        }

        const ResetPayload mapPayload() const {
            return this->value("map").value<ResetPayload>();
        }

        void setStreamState(const StreamPlayStateTracker &state) {
            this->insert("ss", QVariant::fromValue(state));
        } 
        
        void setMapPayload(const ResetPayload &mapPayload) {
            this->insert("map", QVariant::fromValue(mapPayload));
        }
    
    private:
        void _setMessages(const RPZMap<RPZMessage> &messages) {
            this->insert("msgs", QVariant::fromValue(messages));
        }

        void _setUsers(const RPZMap<RPZUser> &users) {
            this->insert("usrs", QVariant::fromValue(users));
        }

        void _defineAsFullSession(bool isFullSession) {
            this->insert("fs", isFullSession);
        }

        void _setSelfUserId(const RPZUser::Id &selfUserId) {
            this->insert("suid", selfUserId);
        }

};