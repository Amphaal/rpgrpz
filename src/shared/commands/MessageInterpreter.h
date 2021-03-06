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

#include <QHash>
#include <QString>

#include <QRegularExpression>
#include <QRandomGenerator>
#include <QPair>

#include <functional>
#include <algorithm>

#include "src/helpers/_appContext.h"

class MessageInterpreter {
 public:
        enum class Command { C_Unknown, Say, Whisper, Help, C_DiceThrow, C_UserLogIn, C_UserLogOut };

        static inline const QHash<QString, Command> _textByCommand = {
            { "/h", Command::Help }
        };

        static Command interpretText(const QString &text);

        static QList<QString> findRecipentsFromText(const QString &text);

        static QString sanitizeText(const QString &text);

        static bool isSendable(const QString &textToSend);

        static QString help();

        static QString usernameToCommandCompatible(const QString &username);

 private:
        static inline const QRegularExpression _hasWhispRegex = QRegularExpression("@(.+?)(?:$|\\s)");
        static inline const QRegularExpression _hasCommandRegex = QRegularExpression("\\/(\\w+)");
};
