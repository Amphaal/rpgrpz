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

#include <QVariantHash>
#include <QMap>

#include "src/_libs/snowflake/snowflake.h"

class Serializable : public QVariantHash {
    
    public:
        Serializable() {};
        explicit Serializable(const QVariantHash &hash) : QVariantHash(hash) {}
        
        Serializable(const SnowFlake::Id &id) {
            this->_setId(id);
        };

        SnowFlake::Id id() const { 
            return this->value(QStringLiteral(u"id")).toULongLong(); 
        };

        QString idAsStr() const { 
            return QString::number(this->id()); 
        };

        void shuffleId() {
            auto id = SnowFlake::get()->nextId();
            this->_setId(id);
        }
    
    private:
        void _setId(const SnowFlake::Id &id) {
            //must be saved as string to prevent parser lack of precision on double conversion
            this->insert(QStringLiteral(u"id"),  QString::number(id));
        }
};