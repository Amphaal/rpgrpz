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

            //user format
            if(auto owner = msg.owner(); owner.id()) _userBoundFormating(layout, msg, owner);
            
            //standard
            else {
                
                switch(msg.commandType()) {
                    case MessageInterpreter::Command::C_DiceThrow: {
                        this->_diceThrowFormating(layout, msg);
                    }
                    break;

                    default: {
                        //normal text
                        auto toAdd = new LogText(msg.toString(), this);
                        layout->addWidget(toAdd);
                    }
                    break;
                }

            }

            this->_defineToolTip(msg);

        }

    private:
        void _defineToolTip(const RPZMessage &msg) {
            auto timestamp = msg.timestamp().toString(QStringLiteral(u"dd.MM.yyyy-hh:mm:ss"));
            this->setToolTip(QObject::tr("Sent at %1").arg(timestamp));
        }

        void _diceThrowFormating(QHBoxLayout* layout, const RPZMessage &msg) {
            
            //CR
            auto cr = new QLabel(QChar(0x2607));
            cr->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Minimum);
            cr->setContentsMargins(0, 0, 5, 0);
                auto crFont = cr->font();
                crFont.setPointSize(15);
                cr->setFont(crFont);
            
            //dice
            auto diceImg = new QLabel(this);
            diceImg->setPixmap(QPixmap(":/icons/app/other/dice.png"));
            diceImg->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Minimum);
            diceImg->setContentsMargins(0, 0, 5, 0);
            
            //throw
            auto result = new LogText(msg.toString(), this);
                auto font = result->font();
                font.setBold(true);
                result->setFont(font);

            //add widgets
            layout->addWidget(cr);
            layout->addWidget(diceImg);
            layout->addWidget(result);

        }

        void _userBoundFormating(QHBoxLayout* layout, const RPZMessage &msg, const RPZUser &owner) {
                
                layout->setContentsMargins(0, 5, 0, 5);

                //companion
                auto companion = new QLabel(this);
                companion->setPixmap(QPixmap(RPZUser::IconsByRoles.value(owner.role())));
                companion->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Minimum);
                companion->setAlignment(Qt::AlignTop);

                //color
                QLabel* colorIndic = nullptr;
                if(auto color = owner.color(); color.isValid()) {
                    
                    colorIndic = new QLabel(this);
                    colorIndic->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Minimum);
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
                name->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Minimum);
                name->setAlignment(Qt::AlignTop);
                name->setContentsMargins(5, 1, 7, 0);
                    auto font = name->font();
                    font.setBold(true);
                    name->setFont(font);

                //text
                QString textStr; 
                switch(msg.commandType()) {
                    
                    case MessageInterpreter::Command::C_UserLogIn:
                        textStr = QObject::tr("logged in !");
                    break;

                    case MessageInterpreter::Command::C_UserLogOut:
                        textStr = QObject::tr("logged out.");
                    break;

                    default: {
                        textStr = msg.toString();
                    }
                    break;

                }
                auto toAdd = new LogText(textStr, this);

            //add widgets
            layout->addWidget(companion);
            if(colorIndic) layout->addWidget(colorIndic);
            layout->addWidget(name);
            layout->addWidget(toAdd);

        }

};
