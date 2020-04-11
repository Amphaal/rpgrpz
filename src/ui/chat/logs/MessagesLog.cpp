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

#include "MessagesLog.h"

MessagesLog::MessagesLog(QWidget *parent) : LogContainer(parent) {
    // this->_getLayout()->setDirection(QBoxLayout::Direction::BottomToTop);
    this->layout()->setAlignment(Qt::AlignBottom);
};

void MessagesLog::handleResponse(const RPZResponse &response) {

    auto respToId = response.answerer();

    //if respond to a message, "ungrey" the responded
    if(respToId) {
        
        auto existingLine = LogContainer::_getLine(respToId);
        auto existingPalette = existingLine->palette();
        
        auto txtColor = existingPalette.color(QPalette::WindowText);
        txtColor.setAlpha(255);
        existingPalette.setColor(QPalette::WindowText, txtColor);

        existingLine->setPalette(existingPalette);

    }

    //if response code is ask, stop here
    auto respCode = response.responseCode();
    if(respCode == RPZResponse::ResponseCode::Ack) return;

    //get new line
    auto newLine = LogContainer::_addLine(response, respToId);
    
    //add text
    auto txt = new LogText(response.toString());
    newLine->horizontalLayout()->addWidget(txt, 10);
    
    //set palette
    newLine->setPalette(response.palette());

}

void MessagesLog::changeEvent(QEvent *event) {
    if(event->type() != QEvent::EnabledChange) return;

    if(!this->isEnabled()) {
        this->clearLines();
    }
}

void MessagesLog::handleNonLocalMessage(const RPZMessage &msg) {
    return this->_handleMessage(msg, false);
}

void MessagesLog::handleLocalMessage(RPZMessage &msg) {
    
    //define as local
    msg.setAsLocal();

    //fill user infos
    if(this->_rpzClient) {
        msg.setOwnership(this->_rpzClient->identity());
    }

    return this->_handleMessage(msg, true);

}

void MessagesLog::_handleMessage(const RPZMessage &msg, bool isLocal) {
    
    //should not exist
    auto targetLine = LogContainer::_getLine(msg);
    if(targetLine) return;

    //create new line
    targetLine = LogContainer::_addLine(msg);
        
    //add content
    auto content = new LogContent(msg);
    targetLine->horizontalLayout()->addWidget(content, 10);

    //define palette to apply
    auto msgPalette = msg.palette();
    
    //if is local, expect a server response and add opacity to signal it to the user
    if(isLocal) {
        auto txtColor = msgPalette.color(QPalette::WindowText);
        txtColor.setAlpha(128);
        msgPalette.setColor(QPalette::WindowText, txtColor);
    }

    //apply it
    targetLine->setPalette(msgPalette);
}