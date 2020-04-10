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

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

#include "src/shared/models/messaging/RPZMessage.h"
#include "LogText.hpp"

class LogContent : public QWidget {

    public:
        LogContent(const RPZMessage &msg) {

            this->setAutoFillBackground(true);
            this->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Maximum);

            this->setLayout(new QVBoxLayout);
            this->layout()->setAlignment(Qt::AlignTop);
            this->setContentsMargins(0, 0, 0, 0);

            switch(msg.commandType()) {
                case MessageInterpreter::Command::C_UserLog: {
                    new LogText(msg.text(), this);
                    //TODO
                }
                break;

                default:
                    new LogText(msg.text(), this);
                break;

            }

            this->_defineToolTip(msg);

        }

    private:
        void _defineToolTip(const RPZMessage &msg) {
            auto timestamp = msg.timestamp().toString(QStringLiteral(u"dd.MM.yyyy-hh:mm:ss"));
            this->setToolTip(QObject::tr("Sent at %1").arg(timestamp));
        }

};
