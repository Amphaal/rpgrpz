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

#include <QDateTime>
#include "Ownable.hpp"

#include <QPalette>

class Stampable : public Ownable {
    public:
        using Id = SnowFlake::Id;

        Stampable() : Ownable(SnowFlake::get()->nextId()) {
            this->_setTimestamp(QDateTime::currentDateTime());
        }
        virtual ~Stampable() {};
        explicit Stampable(const QVariantHash &hash) : Ownable(hash) {}

        QDateTime timestamp() const {
            return this->value(QStringLiteral(u"dt")).toDateTime();
        }

        virtual QString toString() const {
            const auto ts = this->timestamp().toString(QStringLiteral(u"dd.MM.yyyy-hh:mm:ss"));
            const auto name = this->owner().name();
            return QStringLiteral(u"[%1] %2").arg(ts).arg(name);
        }

        virtual QPalette palette() const {
            QPalette palette;
            palette.setColor(QPalette::Window, "#fcfcfc");
            palette.setColor(QPalette::WindowText, "#999999");
            return palette;
        }

    private:
        void _setTimestamp(const QDateTime &dt) {
            this->insert(QStringLiteral(u"dt"), dt);
        }

};