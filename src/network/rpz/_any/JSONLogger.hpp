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

#pragma once 

#include <QString>
#include <QDebug>
#include "RPZJSON.hpp"

class JSONLogger {
    public:
        JSONLogger(const QString &loggerId) : _loggerId(loggerId) {};
    
        void log(const RPZJSON::Method &method, const QString &msg) {
            qDebug() << qUtf8Printable(this->_loggerId)
                        << "-" 
                        << method
                        << "->" 
                        << qUtf8Printable(msg);
        }

        void log(const QString &msg) {
            qDebug() << qUtf8Printable(this->_loggerId)
                     << "-" 
                     << qUtf8Printable(msg);
        }


    private:
        QString _loggerId;
};