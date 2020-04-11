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
// different license and copyright still refer to this GNU General Public License.

#include "MessageInterpreter.h"

MessageInterpreter::Command MessageInterpreter::interpretText(const QString &text) {
    //initial trim
    auto cp_text = text;
    cp_text = cp_text.trimmed();
    
    //if has whisper command
    if(_hasWhispRegex.match(cp_text).hasMatch()) return Command::Whisper;

    //if not a command
    if(!cp_text.startsWith("/")) return Command::Say;

    //extract command
    auto command = cp_text.split(" ", QString::SplitBehavior::SkipEmptyParts).value(0);

    //returns command   
    return _textByCommand.value(command);
}

void MessageInterpreter::generateValuesOnDiceThrows(QVector<DiceThrow> &throws) {
    
    for(auto &dThrow : throws) {
        
        double sum = 0;
        QVector<uint> sout;
        
        for(uint i = 1; i <= dThrow.howMany; i++) {
            auto rand = QRandomGenerator::global()->bounded((uint)1, dThrow.face + 1); //generate throw
            sout += rand; //add value
            sum += rand; //sum
        }

        dThrow.values = sout; //stores all values
        if(dThrow.howMany > 0) dThrow.avg = sum / dThrow.howMany; //calculate avg
        
        //regroup throws
        QMultiHash<uint, bool> buf;
        for(const auto i : sout) {
            buf.insertMulti(i, false);
        }

        //order keys desc
        auto keys = buf.uniqueKeys();
        std::sort(keys.begin(), keys.end(), std::greater<int>());

        QVector<QPair<uint, int>> out;
        for(const auto face : keys) {
            auto count = buf.values(face).count();
            out += { face, count };
        }
        dThrow.pairedValues = out;

    }

}

QVector<DiceThrow> MessageInterpreter::findDiceThrowsFromText(const QString &text) {
    QVector<DiceThrow> out;

    auto matches = _mustLaunchDice.globalMatch(text);
    while (matches.hasNext()) {
        auto match = matches.next(); //next
        
        auto face = match.captured(2).toUInt();
        if(face < 2) continue;

        auto howMany = match.captured(1).toUInt();
        auto name = match.captured(0);

        DiceThrow diceThrow;
        diceThrow.howMany = howMany;
        diceThrow.face = face;
        diceThrow.name = name;

        out += diceThrow;
    }

    return out;
}

QList<QString> MessageInterpreter::findRecipentsFromText(const QString &text) {
    
    auto matches = _hasWhispRegex.globalMatch(text);
    QSet<QString> out;

    //iterate
    while (matches.hasNext()) {
        auto match = matches.next(); //next
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
    
    //if has whisper
    if(_hasWhispRegex.match(textToSend).hasMatch()) {
        auto q = sanitizeText(textToSend);
        return !q.isEmpty();
    }

    return !textToSend.trimmed().isEmpty();
}

QString MessageInterpreter::help()  {
    QString help;
    help += QObject::tr("Server %1 :\n").arg(APP_FULL_DENOM);
    help += QObject::tr("- To whisper : @{toWhisperTo}.\n");
    help += QObject::tr("- To throw dices : {numberOfThrows}D{diceFace}. Exemple : 1d5, 2D20, 3D16.");
    return help;
}

QString MessageInterpreter::usernameToCommandCompatible(const QString &username) {
    auto cp_username = username;
    cp_username = cp_username.toLower();
    cp_username = cp_username.replace(" ", "_");
    return cp_username;
}
