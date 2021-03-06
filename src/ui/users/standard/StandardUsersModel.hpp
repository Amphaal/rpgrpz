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

#pragma once

#include "src/ui/users/_base/BaseUsersModel.hpp"

class StandardUsersModel : public BaseUsersModel {
 public:
    StandardUsersModel() { }

    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || !this->_users.count()) return QVariant();

        // check if targeted user exists
        auto expectedId = this->_users.keys().at(index.row());
        auto user = this->_users.value(expectedId);
        if (user.isEmpty()) return QVariant();

        switch (role) {
            case Qt::DisplayRole: {
                return user.name();
            }

            case Qt::DecorationRole: {
                auto role = user.role();
                auto path = RPZUser::IconsByRoles.value(role);
                return QIcon(path);
            }
        }

        return QVariant();
    }

 private:
    bool _isUserInvalidForInsert(const RPZUser &user) const override {
        return user.role() == RPZUser::Role::Player;
    }
};
