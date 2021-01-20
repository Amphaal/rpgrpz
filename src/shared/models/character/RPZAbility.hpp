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

#include <QVariantHash>

class RPZAbility : public QVariantHash {
 public:
        RPZAbility() {}
        explicit RPZAbility(const QVariantHash &hash) : QVariantHash(hash) {}

        void setName(const QString &name) {this->insert(QStringLiteral(u"n"), name);}
        const QString name() const {return this->value(QStringLiteral(u"n")).toString();}

        void setCategory(const QString &archtype) { this->insert(QStringLiteral(u"c"), archtype); }
        const QString category() const {return this->value(QStringLiteral(u"c")).toString();}

        void setDescription(const QString &descr) { this->insert(QStringLiteral(u"d"), descr); }
        const QString description() const {return this->value(QStringLiteral(u"d")).toString();}

        void setFavorite(const bool &isFavorite) {this->insert(QStringLiteral(u"f"), isFavorite);}
        bool isFavorite() const { return this->value(QStringLiteral(u"f")).toBool();}
};