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

#include <QList>

#include "Serializable.hpp"

template<class T>
class RPZMap : public QMap<SnowFlake::Id, T> {

    static_assert(std::is_base_of<Serializable, T>::value, "Must derive from Serializable");
    T element;
    
 public:
        RPZMap() {}    
        ~RPZMap() {}   

        RPZMap(const QList<T> &serializables) {
            for(const auto &s : serializables) {
                this->insert(s.id(), s);
            }
        }

        RPZMap(const T &serializable) {
            this->insert(serializable.id(), serializable);
        }

        QVariantList toVList() const {
            QVariantList out;
            for(const auto &base : this->values()) {
                out.append(base);
            }
            return out;
        }

        QVariantMap toVMap() const {
            QVariantMap out;
            for(auto i = this->begin(); i != this->end(); i++) {
                out.insert(QString::number(i.key()), i.value());
            }
            return out;
        }

        static RPZMap<T> fromVMap(const QVariantMap &map) {
            
            RPZMap<T> out;
            
            for(auto i = map.constBegin(); i != map.constEnd(); i++) {
                auto id = i.key().toULongLong();
                T casted(i.value().toHash());
                out.insert(id, casted);
            }

            return out;

        }

};