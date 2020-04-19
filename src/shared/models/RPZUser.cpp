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

#include "RPZUser.h"

RPZUser::RPZUser() {}
RPZUser::RPZUser(const QVariantHash &hash) : Serializable(hash) {}

RPZUser::RPZUser(RPZUser::Id id, const QString &name, const Role &role, const QColor &color) : Serializable(id) {
    this->setName(name);
    this->setRole(role);
    this->_setColor(color);
}

QString RPZUser::idAsBase62() const {
    auto value = this->id();
    QString out;

    do {
        auto dd = std::string(1, _CODES_b62[value % 62]);
        out.insert(0, QString::fromStdString(dd));
        value /= 62;
    } while (value > 0);

    return out;
}

void RPZUser::setName(const QString &name) {
    // default name
    this->insert(QStringLiteral(u"name"), name);

    // whisp name
    auto adapted = MessageInterpreter::usernameToCommandCompatible(name);
    adapted = adapted + "#" + this->idAsBase62();
    this->insert(QStringLiteral(u"wname"), adapted);
}

void RPZUser::setRole(const Role &role) {
    this->insert(QStringLiteral(u"role"), (int)role);
}

QString RPZUser::name() const {
    auto name = this->value(QStringLiteral(u"name")).toString();
    if (!name.isEmpty()) return name;

    return NULL;
}

RPZUser::Role RPZUser::role() const {
    return (Role)this->value(QStringLiteral(u"role"), 0).toInt();
}

QColor RPZUser::color() const {
    auto colorAsStr = this->value(QStringLiteral(u"color")).toString();
    return colorAsStr.isEmpty() ? QColor() : QColor(colorAsStr);
}

QString RPZUser::toString() const {
    auto name = this->name();
    if (!name.isNull()) {
        return this->name();
    }

    if (auto id = this->id()) {
        return QString::number(id);
    }

    return "Moi";
}

QString RPZUser::whisperTargetName() const {
    return this->value(QStringLiteral(u"wname")).toString();
}

void RPZUser::randomiseColor() {
    this->_setColor();
}

void RPZUser::_setColor(const QColor &color) {
    auto colorToUse = color.isValid() ? color.name() : RandomColor::getRandomKellyColor(RandomColor::Context::PlayerColor).name();
    this->insert(QStringLiteral(u"color"), colorToUse);
}

void RPZUser::setCharacter(const RPZCharacter &character) {
    this->insert(QStringLiteral(u"char"), character);
}

const RPZCharacter RPZUser::character() const {
    return RPZCharacter(this->value(QStringLiteral(u"char")).toHash());
}
