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

#include <QAbstractListModel>
#include <QVector>
#include <QIcon>

#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/ui/_others/ConnectivityObserver.h"

class BaseUsersModel : public QAbstractListModel, public ConnectivityObserver {
    
    public:
        BaseUsersModel() { };

        int rowCount(const QModelIndex &parent) const override {
            
            if(parent.isValid()) return 0;

            auto count = this->_users.count();
            return count;

        }

    protected:
        RPZMap<RPZUser> _users;

        virtual bool _isUserInvalidForInsert(const RPZUser &user) const = 0;

        void connectionClosed(bool hasInitialMapLoaded) override {
            
            this->beginResetModel();
                this->_users.clear();
            this->endResetModel();

        }

        void connectingToServer() override {
            
            //update all users
            QObject::connect(
                this->_rpzClient, &RPZClient::gameSessionReceived,
                this, &BaseUsersModel::_onGameSessionReceived
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

            //on remote user data changed
            QObject::connect(
                this->_rpzClient, &RPZClient::userDataChanged,
                this, &BaseUsersModel::_onUserDataChanged
            );

        }

    private:
        int _getRow(const RPZUser &user) {
            return this->_users.keys().indexOf(user.id());
        }

        int _anticipateRow(const RPZUser::Id &userId) {
            
            auto keys = this->_users.keys();
            
            auto foundExact = keys.indexOf(userId);
            if(foundExact > -1) return foundExact;

            keys.append(userId);
            std::sort(keys.begin(), keys.end());

            auto foundInserted = keys.indexOf(userId);
            if(foundInserted > -1) return foundInserted;

            return 0;

        }

        void _onGameSessionReceived(const RPZGameSession &gameSession) {          
            
            this->beginResetModel();

                this->_users.clear();
                
                for(const auto &user : gameSession.users()) {
                    if(this->_isUserInvalidForInsert(user)) continue;
                    this->_users.insert(user.id(), user);
                }

            this->endResetModel();

        }

        void _onUserJoinedServer(const RPZUser &newUser) {
            if(this->_isUserInvalidForInsert(newUser)) return;
            
            auto prevision = this->_anticipateRow(newUser.id());

            this->beginInsertRows(QModelIndex(), prevision, prevision);
                this->_users.insert(newUser.id(), newUser);
            this->endInsertRows();

        }

        void _onUserLeftServer(const RPZUser &userOut) {
            
            auto idOut = userOut.id();
            if(!this->_users.contains(idOut)) return;
            
            auto toRemove = this->_getRow(userOut);

            this->beginRemoveRows(QModelIndex(), toRemove, toRemove);
                this->_users.remove(idOut);
            this->endRemoveRows();

        }

        void _onUserDataChanged(const RPZUser &updated) {
            
            if(this->_isUserInvalidForInsert(updated)) return;

            auto userIndex = this->index(this->_getRow(updated), 0);
            this->_users.insert(updated.id(), updated); //update existing
            emit dataChanged(userIndex, userIndex);

        }
};