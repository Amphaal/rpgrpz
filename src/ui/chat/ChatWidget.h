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

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QStyle>
#include "src/ui/_others/RestoringSplitter.h"

#include "src/shared/models/messaging/RPZMessage.h"
#include "src/shared/models/messaging/RPZResponse.h"

#include "src/shared/models/RPZUser.h"
#include "src/ui/_others/ConnectivityObserver.h"

#include "ChatEdit.h"

#include "src/ui/chat/logs/_base/LogScrollView.h"
#include "src/ui/chat/logs/MessagesLog.h"


class ChatWidget : public QWidget, public ConnectivityObserver {
    Q_OBJECT

 public:
    explicit ChatWidget(QWidget *parent = nullptr);
    MessagesLog* messageLog();

 protected:
     void connectingToServer() override;
     void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override;

 private slots:
    void _onRPZClientEnded(const QString &statusMsg);
    void _onGameSessionReceived(const RPZGameSession &gameSession);
    void _onMessageReceived(const RPZMessage &message);
    void _onServerResponseReceived(const RPZResponse &reponse);

 private:
    MessagesLog *_chatLog;
    ChatEdit *_chatEdit;

    QString serverName;

    void _sendMessage();

    // ui helpers
    void _instUI();
};
