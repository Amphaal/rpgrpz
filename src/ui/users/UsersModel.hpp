#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QIcon>

#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/ui/_others/ClientBindable.h"

class UsersModel : public QAbstractListModel, public ClientBindable {
    
    public:
        UsersModel() { };

        QVariant data(const QModelIndex &index, int role) const override {
            
            if (!index.isValid() || !this->_users.count()) return QVariant();
            
            auto expectedId = this->_users.keys().at(index.row());
            auto user = this->_users.value(expectedId);
            if(user.isEmpty()) return QVariant();

            switch(role) {

                case Qt::UserRole: {
                    return user;
                }

            }

            return QVariant();

        }

        int rowCount(const QModelIndex &parent) const override {
            if(parent.isValid()) return 0;
            return this->_users.count();
        }

    protected:
        void onRPZClientDisconnect() override {
            this->beginResetModel();
                this->_users.clear();
            this->endResetModel();
        }

        void onRPZClientConnecting() override {
            
            //update all users
            QObject::connect(
                this->_rpzClient, &RPZClient::allUsersReceived,
                this, &UsersModel::_onAllUsersReceived
            );

            //on new user
            QObject::connect(
                this->_rpzClient, &RPZClient::userJoinedServer,
                this, &UsersModel::_onUserJoinedServer
            );

            //on user leaving
            QObject::connect(
                this->_rpzClient, &RPZClient::userLeftServer,
                this, &UsersModel::_onUserLeftServer
            );

        }

    private:
        RPZMap<RPZUser> _users;

        void _onAllUsersReceived() {
            this->beginResetModel();
                this->_users = this->_rpzClient->sessionUsers();
            this->endResetModel();
        }

        void _onUserJoinedServer(const RPZUser &newUser) {
            this->beginResetModel();
                this->_users.insert(newUser.id(), newUser);
            this->endResetModel();
        }

        void _onUserLeftServer(snowflake_uid userId) {
            this->beginResetModel();
                this->_users.remove(userId);
            this->endResetModel();
        }

};