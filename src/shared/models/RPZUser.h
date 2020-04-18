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
 
#include <QString>
#include <QColor>
#include <QVariantHash>
#include <QHostAddress>

#include "src/shared/models/_base/RPZMap.hpp"
#include "src/helpers/RandomColor.h"
#include "src/shared/models/_base/Serializable.hpp"
#include "src/shared/commands/MessageInterpreter.h"

#include "src/shared/models/character/RPZCharacter.hpp"

class RPZUser : public Serializable {
     public:
        using Id = SnowFlake::Id;

        enum class Role { Observer, Host, Player };

        static const inline QHash<RPZUser::Role, QString> IconsByRoles { 
            { Role::Observer, QStringLiteral(u":/icons/app/connectivity/observer.png") },
            { Role::Host, QStringLiteral(u":/icons/app/connectivity/crown.png") },
            { Role::Player, QStringLiteral(u":/icons/app/connectivity/cloak.png") }
        };
        
        RPZUser();
        explicit RPZUser(const QVariantHash &hash);
        RPZUser(RPZUser::Id id, const QString &name, const Role &role, const QColor &color);

        void setName(const QString &name);
        void setRole(const Role &role);
        void setCharacter(const RPZCharacter &character);
        void randomiseColor();

        QString idAsBase62() const;

        QString name() const;
        QString whisperTargetName() const;
        Role role() const;
        QColor color() const;
        QString toString() const;
        const RPZCharacter character() const;

    private:
        void _setColor(const QColor &color = QColor());

        static inline std::string _CODES_b62 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

};

inline uint qHash(const RPZUser::Role &key, uint seed = 0) {return uint(key) ^ seed;}