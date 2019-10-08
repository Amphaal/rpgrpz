#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QIcon>

#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/ui/_others/ClientBindable.h"

class BaseUsersModel : public QAbstractListModel, public ClientBindable {
    
    public:
        BaseUsersModel() { };

        int rowCount(const QModelIndex &parent) const override {
            if(parent.isValid()) return 0;
            return this->_users.count();
        }

    protected:
        RPZMap<RPZUser> _users;

        virtual bool _isUserInvalidForInsert(const RPZUser &user) = 0;

        void onRPZClientDisconnect() override {
            this->beginResetModel();
                this->_users.clear();
            this->endResetModel();
        }

        void onRPZClientConnecting() override {
            
            //update all users
            QObject::connect(
                this->_rpzClient, &RPZClient::allUsersReceived,
                this, &BaseUsersModel::_onAllUsersReceived
            );

            //on new user
            QObject::connect(
                this->_rpzClient, &RPZClient::userJoinedServer,
                this, &BaseUsersModel::_onUserJoinedServer
            );

            //on user leaving
            QObject::connect(
                this->_rpzClient, &RPZClient::userLeftServer,
                this, &BaseUsersModel::_onUserLeftServer
            );

        }

    private:
        void _onAllUsersReceived() {          
            this->beginResetModel();

                this->_users.clear();
                for(auto &user : this->_rpzClient->sessionUsers()) {
                    if(this->_isUserInvalidForInsert(user)) continue;
                    this->_users.insert(user.id(), user);
                }

            this->endResetModel();
        }

        void _onUserJoinedServer(const RPZUser &newUser) {
            if(this->_isUserInvalidForInsert(newUser)) return;
            
            this->beginResetModel();

                this->_users.insert(newUser.id(), newUser);

            this->endResetModel();
        }

        void _onUserLeftServer(snowflake_uid userId) {
            if(!this->_users.contains(userId)) return;
            
            this->beginResetModel();
                this->_users.remove(userId);
            this->endResetModel();
        }

};