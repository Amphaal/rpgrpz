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
#include <QHBoxLayout>

#include "src/shared/models/messaging/RPZMessage.h"
#include "src/ui/audio/NotificationsAudioManager.hpp"
#include "LogText.hpp"

class LogContent : public QWidget {

    public:
        LogContent(const RPZMessage &msg) {

            this->setAutoFillBackground(true);
            this->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Maximum);
            this->setContentsMargins(0, 0, 0, 0);

            auto layout = new QHBoxLayout;
            this->setLayout(layout);            
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->setAlignment(Qt::AlignTop);

            auto cmdType = msg.commandType();

            //user format
            if(cmdType == MessageInterpreter::Command::C_DiceThrow) _diceThrowFormating(layout);
            if(auto owner = msg.owner(); owner.id()) _userBoundFormating(layout, msg, owner);
            
            //log text
            QString textStr; 
            switch(cmdType) {
                
                case MessageInterpreter::Command::C_UserLogIn:
                    textStr = QObject::tr("logged in !");
                break;

                case MessageInterpreter::Command::C_UserLogOut:
                    textStr = QObject::tr("logged out.");
                break;

                default:
                    textStr = msg.toString();
                break;

            }
            auto logText = new LogText(textStr, this);

            layout->addWidget(logText);
            this->_defineToolTip(msg);

        }

    private:
        void _defineToolTip(const RPZMessage &msg) {
            auto timestamp = msg.timestamp().toString(QStringLiteral(u"dd.MM.yyyy-hh:mm:ss"));
            this->setToolTip(QObject::tr("Sent at %1").arg(timestamp));
        }

        void _diceThrowFormating(QHBoxLayout* layout) {
            
            //dice
            auto diceImg = new QLabel(this);
            diceImg->setPixmap(QPixmap(":/icons/app/other/dice.png"));
            diceImg->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Minimum);
            diceImg->setContentsMargins(0, 0, 5, 0);
            
            //add widgets
            layout->addWidget(diceImg);

            NotificationsAudioManager::get()->playDiceThrow();

        }

    void _userBoundFormating(QHBoxLayout* layout, const RPZMessage &msg, const RPZUser &owner) {
                
                layout->setContentsMargins(0, 5, 0, 5);

                //companion
                auto companion = new QLabel(this);
                companion->setPixmap(QPixmap(RPZUser::IconsByRoles.value(owner.role())));
                companion->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
                companion->setAlignment(Qt::AlignTop);

                //color
                QLabel* colorIndic = nullptr;
                if(auto color = owner.color(); color.isValid()) {
                    
                    colorIndic = new QLabel(this);
                    colorIndic->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
                    colorIndic->setAlignment(Qt::AlignTop);

                    colorIndic->setFixedWidth(10);
                    colorIndic->setFixedHeight(10);
                    colorIndic->setFrameShape(QFrame::Panel);
                    colorIndic->setLineWidth(1);
                    colorIndic->setAutoFillBackground(true);
                    
                    QPalette palette;
                    palette.setColor(QPalette::Window, color);
                    colorIndic->setPalette(palette);
                    
                } 

                //name
                auto name = new QLabel(owner.toString(), this);
                name->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
                name->setAlignment(Qt::AlignTop);
                name->setContentsMargins(5, 0, 7, 1);
                    auto font = name->font();
                    font.setBold(true);
                    name->setFont(font);

            //add widgets
            layout->addWidget(companion);
            if(colorIndic) layout->addWidget(colorIndic);
            layout->addWidget(name);

        }

};
