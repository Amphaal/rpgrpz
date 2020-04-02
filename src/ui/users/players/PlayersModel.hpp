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

#pragma once

#include "src/ui/users/_base/BaseUsersModel.hpp"
#include "PlayerItemDelegate.hpp"

class PlayersModel : public BaseUsersModel {
    
    Q_OBJECT

    public:
        PlayersModel() { };

        QVariant data(const QModelIndex &index, int role) const override {
            
            if (!index.isValid() || !this->_users.count()) return QVariant();
            
            if(role == Qt::SizeHintRole) return PlayerItemDelegate::sizeHint(index);

            //check if targeted user exists
            auto expectedId = this->_users.keys().at(index.row());
            
            auto user = this->_users.value(expectedId);
            if(user.isEmpty()) return QVariant();

            switch(role) {

                case Qt::ToolTipRole: {
                    auto character = user.character();
                    return tr("[%1] as \"%2\"").arg(user.name()).arg(character.name());
                }

                case Qt::UserRole: {
                    return user;
                }

            }

            return QVariant();

        }

    private:
        bool _isUserInvalidForInsert(const RPZUser &user) const override {
            return user.role() != RPZUser::Role::Player;
        }
};