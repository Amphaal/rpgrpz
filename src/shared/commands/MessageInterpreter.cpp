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

#include "MessageInterpreter.h"

MessageInterpreter::Command MessageInterpreter::interpretText(const QString &text) {
    // initial trim
    auto cp_text = text;
    cp_text = cp_text.trimmed();

    // if has whisper command
    if (_hasWhispRegex.match(cp_text).hasMatch()) return Command::Whisper;

    // if not a command
    if (!cp_text.startsWith("/")) return Command::Say;

    // extract command
    auto command = cp_text.split(" ", Qt::SkipEmptyParts).value(0);

    // returns command
    return _textByCommand.value(command);
}

QList<QString> MessageInterpreter::findRecipentsFromText(const QString &text) {
    auto matches = _hasWhispRegex.globalMatch(text);
    QSet<QString> out;

    // iterate
    while (matches.hasNext()) {
        auto match = matches.next();  // next
        out.insert(match.captured(1));
    }

    return out.values();
}

QString MessageInterpreter::sanitizeText(const QString &text) {
    auto cp_text = text;
    cp_text.remove(_hasWhispRegex);
    cp_text.remove(_hasCommandRegex);
    return cp_text.trimmed();
}

bool MessageInterpreter::isSendable(const QString &textToSend) {
    // if has whisper
    if (_hasWhispRegex.match(textToSend).hasMatch()) {
        auto q = sanitizeText(textToSend);
        return !q.isEmpty();
    }

    return !textToSend.trimmed().isEmpty();
}

QString MessageInterpreter::help()  {
    QString help;
    help += QObject::tr("Server %1 :").arg(APP_FULL_DENOM) + "<br/>";
    help += QObject::tr("- To whisper : @{toWhisperTo}.") + "<br/>";
    help += QObject::tr("- To throw dices : {DicerThrowCommand}. Please visit https://github.com/Amphaal/Dicer/blob/master/tests/tests.cpp for more infos.");
    return help;
}

QString MessageInterpreter::usernameToCommandCompatible(const QString &username) {
    auto cp_username = username;
    cp_username = cp_username.toLower();
    cp_username = cp_username.replace(" ", "_");
    return cp_username;
}
