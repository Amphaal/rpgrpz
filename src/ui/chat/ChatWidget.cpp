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

#include "ChatWidget.h"

ChatWidget::ChatWidget(QWidget *parent) : QWidget(parent), _chatLog(new MessagesLog),  _chatEdit(new ChatEdit) {
    this->setEnabled(false);

    // UI...
    this->_instUI();
}

MessagesLog* ChatWidget::messageLog() {
    return this->_chatLog;
}

void ChatWidget::_instUI() {
    auto layout = new QVBoxLayout;
    layout->setMargin(0);

    auto scroller = new LogScrollView;
    scroller->setWidget(this->_chatLog);

    layout->addWidget(scroller);
    layout->addWidget(this->_chatEdit);

    this->setLayout(layout);
    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
}

void ChatWidget::_onRPZClientEnded(const QString &statusMsg) {
    if(statusMsg.isEmpty()) return;

    // out log
    RPZResponse response(0, RPZResponse::ResponseCode::Error, statusMsg);
    this->_chatLog->handleResponse(response);

    this->setEnabled(false);
}

void ChatWidget::_onGameSessionReceived(const RPZGameSession &gameSession) {
    this->setEnabled(true);

    // add list of messages
    for (const auto &msg : gameSession.messages()) {
        this->_chatLog->handleHistoryMessage(msg);
    }

    // welcome msg
    auto response = RPZResponse(0, RPZResponse::ResponseCode::ConnectedToServer, this->serverName);
    this->_chatLog->handleResponse(response);
}

void ChatWidget::connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) {
    this->_onRPZClientEnded(tr("Disconnected from server"));
}

void ChatWidget::connectingToServer() {
    this->serverName = _rpzClient->getConnectedSocketAddress();

    this->_chatLog->clearLines();

    // on error from client
    QObject::connect(
        _rpzClient, &RPZClient::ended,
        this, &ChatWidget::_onRPZClientEnded
    );

    // welcome once all history have been received
    QObject::connect(
        _rpzClient, &RPZClient::gameSessionReceived,
        this, &ChatWidget::_onGameSessionReceived
    );

    // on message received
    QObject::connect(
        _rpzClient, &RPZClient::receivedMessage,
        this, &ChatWidget::_onMessageReceived
    );

    // on server response
    QObject::connect(
        _rpzClient, &RPZClient::serverResponseReceived,
        this, &ChatWidget::_onServerResponseReceived
    );

    // on message send request
    this->_chatEdit->disconnect();  // disconnect previous message sending handling
    QObject::connect(
        this->_chatEdit, &ChatEdit::askedToSendCommand,
        [=](const QString &msg, bool isDiceThrowCommand) {
            // generate message
            RPZMessage message(msg);
            if(isDiceThrowCommand) message.setAsDiceThrowCommand();

            // handle
            this->_chatLog->handleLocalMessage(message);
            QMetaObject::invokeMethod(this->_rpzClient, "sendMessage", Q_ARG(RPZMessage, message));
    });
}

void ChatWidget::_onMessageReceived(const RPZMessage &message) {
    this->_chatLog->handleRemoteMessage(message);
}

void ChatWidget::_onServerResponseReceived(const RPZResponse &reponse) {
    this->_chatLog->handleResponse(reponse);
}
