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

#include "MessagesLog.h"

MessagesLog::MessagesLog(QWidget *parent) : LogContainer(parent) {
    // this->_getLayout()->setDirection(QBoxLayout::Direction::BottomToTop);
    this->layout()->setAlignment(Qt::AlignBottom);
}

void MessagesLog::handleResponse(const RPZResponse &response) {
    auto respToId = response.answerer();

    // if respond to a message, "ungrey" the responded
    if (respToId) {
        auto existingLine = LogContainer::_getLine(respToId);
        auto existingPalette = existingLine->palette();

        auto txtColor = existingPalette.color(QPalette::WindowText);
        txtColor.setAlpha(255);
        existingPalette.setColor(QPalette::WindowText, txtColor);

        existingLine->setPalette(existingPalette);
    }

    // if response code is ask, stop here
    auto respCode = response.responseCode();
    if (respCode == RPZResponse::ResponseCode::Ack) return;

    // get new line
    auto newLine = LogContainer::_addLine(response, respToId);

    // add text
    auto txt = new LogText(response.toString());
    newLine->horizontalLayout()->addWidget(txt, 10);

    // set palette
    newLine->setPalette(response.palette());
}

void MessagesLog::changeEvent(QEvent *event) {
    if (event->type() != QEvent::EnabledChange) return;

    if (!this->isEnabled()) {
        this->clearLines();
    }
}

void MessagesLog::handleRemoteMessage(const RPZMessage &msg) {
    // handle message
    this->_handleMessage(msg, false, false);

    // remote-only notifications
    switch (msg.commandType()) {
        case MessageInterpreter::Command::Whisper:
            NotificationsAudioManager::get()->playWhisper();
        break;

        default:
        break;
    }
}

void MessagesLog::handleHistoryMessage(const RPZMessage &msg) {
    return this->_handleMessage(msg, false, true);
}

void MessagesLog::handleLocalMessage(RPZMessage &msg) {
    // define as local
    msg.setAsLocal();

    // fill user infos
    if (this->_rpzClient) {
        msg.setOwnership(this->_rpzClient->identity());
    }

    return this->_handleMessage(msg, true);
}

void MessagesLog::_handleMessage(const RPZMessage &msg, bool isLocal, bool fromHistory) {
    // should not exist
    auto targetLine = LogContainer::_getLine(msg);
    if (targetLine) return;

    // create new line
    targetLine = LogContainer::_addLine(msg);

    // add content
    auto content = new LogContent(msg);
    targetLine->horizontalLayout()->addWidget(content, 10);

    // define palette to apply
    auto msgPalette = msg.palette();

    // if is local, expect a server response and add opacity to signal it to the user
    if (isLocal) {
        auto txtColor = msgPalette.color(QPalette::WindowText);
        txtColor.setAlpha(128);
        msgPalette.setColor(QPalette::WindowText, txtColor);
    }

    // play sound
    if (!fromHistory) {
        switch (msg.commandType()) {
            case MessageInterpreter::Command::C_DiceThrow:
                NotificationsAudioManager::get()->playDiceThrow();
            break;

            default:
            break;
        }
    }

    // apply palette
    targetLine->setPalette(msgPalette);

    // tag as not seen
    if (!this->_isVisibleByUser) {
        this->_msgIdsNotSeen.append(msg.id());
        auto unseenMessagesCount = this->_msgIdsNotSeen.count();
        emit notificationCountUpdated(unseenMessagesCount);
    }
}

void MessagesLog::setIsUserVisible(bool isVisibleByUser) {
    this->_isVisibleByUser = isVisibleByUser;
    if(this->_isVisibleByUser) {
        this->_msgIdsNotSeen.clear();
        emit notificationCountUpdated(0);
    }
}
