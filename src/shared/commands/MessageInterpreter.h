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

#include <QHash>
#include <QString>

#include <QRegularExpression>
#include <QRandomGenerator>
#include <QPair>

#include "src/helpers/_appContext.h"

struct DiceThrow {
    uint face = 0;
    uint howMany = 0;
    QVector<uint> values;
    QVector<QPair<uint, int>> pairedValues;
    QString name;
    double avg = 0;
};

class MessageInterpreter {
    public:
        enum class Command { C_Unknown, Say, Whisper, Help, C_DiceThrow, C_UserLog };
        
        static inline const QHash<QString, Command> _textByCommand = {
            { "/h", Command::Help }
        };

        static Command interpretText(const QString &text);

        static QList<QString> findRecipentsFromText(const QString &text);

        static QString sanitizeText(const QString &text);

        static bool isSendable(const QString &textToSend);

        static QString help();

        static QString usernameToCommandCompatible(const QString &username);

        static QVector<DiceThrow> findDiceThrowsFromText(const QString &text);
        static void generateValuesOnDiceThrows(QVector<DiceThrow> &throws);

    private:
        static inline const QRegularExpression _hasWhispRegex = QRegularExpression("@(.+?)(?:$|\\s)");
        static inline const QRegularExpression _hasCommandRegex = QRegularExpression("\\/(\\w+)");
        static inline const QRegularExpression _mustLaunchDice = QRegularExpression("([1-9]|1[0-6])[dD](\\d+)");
};