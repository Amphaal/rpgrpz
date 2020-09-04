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

#include "RPZMessage.h"

RPZMessage::RPZMessage() {}
RPZMessage::~RPZMessage() {}

RPZMessage::RPZMessage(const QVariantHash &hash) : Stampable(hash) {
    this->_interpretTextAsCommand();
}

RPZMessage::RPZMessage(const QString &message, const MessageInterpreter::Command &forceCommand) : Stampable() {
    if ((int)forceCommand) this->_forceCommand(forceCommand);
    this->_setText(message);
}

void RPZMessage::setAsDiceThrowCommand() {
    this->insert(QStringLiteral(u"dtc"), true);
}

bool RPZMessage::isDiceThrowCommand() const {
    return this->value(QStringLiteral(u"dtc")).toBool();
}

QString RPZMessage::text() const {
    return this->value(QStringLiteral(u"txt")).toString();
}

MessageInterpreter::Command RPZMessage::commandType() const {
    return this->_command;
}

void RPZMessage::setDiceThrowResult(double result) {
    this->insert(QStringLiteral(u"dtr"), result);
}

bool RPZMessage::haveDiceThrowResult() const {
    return this->contains(QStringLiteral(u"dtr"));
}

double RPZMessage::diceThrowResult() const {
    return this->value(QStringLiteral(u"dtr")).toDouble();
}

void RPZMessage::setAsLocal() {
    this->_isLocal = true;
}

QString RPZMessage::toString() const {
    auto base = Stampable::toString();
    auto text = this->text();

    switch (this->_command) {
        case MessageInterpreter::Command::Whisper: {
            auto textPrefix = QObject::tr(" whispers to you : ");

            if (this->_isLocal) {
                auto recipientList = MessageInterpreter::findRecipentsFromText(text).join(", ");
                textPrefix = QObject::tr(" whisper to ") + recipientList + " : ";
                text = MessageInterpreter::sanitizeText(text);
            }

            return textPrefix + text;
        }
        break;

        default:
            return text;
    }

    return QString();
}

QPalette RPZMessage::palette() const {
    // default palette
    auto palette = Stampable::palette();

    // switch by resp code...
    switch (this->_command) {
        case MessageInterpreter::Command::Whisper:
            palette.setColor(QPalette::Window, "#f2e8f9");
            palette.setColor(QPalette::WindowText, "#a12ded");
            break;

        case MessageInterpreter::Command::Say:
            palette.setColor(QPalette::Window, "#FFFFFF");
            palette.setColor(QPalette::WindowText, "#000000");
            break;

        case MessageInterpreter::Command::C_DiceThrow:
            palette.setColor(QPalette::Window, "#87CEEB");
            palette.setColor(QPalette::WindowText, "#000080");
            break;

        case MessageInterpreter::Command::C_UserLogOut:
        case MessageInterpreter::Command::C_UserLogIn:
            palette.setColor(QPalette::Window, "#e3dc0b");
            palette.setColor(QPalette::WindowText, "#000000");
            break;

        default:
            break;
    }

    return palette;
}

void RPZMessage::_setText(const QString &text) {
    this->insert(QStringLiteral(u"txt"), text);
    this->_interpretTextAsCommand();
}

void RPZMessage::_forceCommand(const MessageInterpreter::Command &forced) {
    this->insert(QStringLiteral(u"cmd"), (int)forced);
}

void RPZMessage::_interpretTextAsCommand() {
    auto forcedCommand = (MessageInterpreter::Command)this->value(QStringLiteral(u"cmd")).toInt();
    this->_command = (int)forcedCommand ? forcedCommand : MessageInterpreter::interpretText(this->text());
}
