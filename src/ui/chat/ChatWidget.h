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

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
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

    public:
        ChatWidget(QWidget *parent = nullptr);

        void connectingToServer() override;

    private:
        MessagesLog *_chatLog;
        ChatEdit *_chatEdit;

        QString serverName;

        void _sendMessage();

        //ui helpers
        void _instUI();

        void _onRPZClientStatus(const QString &statusMsg, bool isError);
        void _onGameSessionReceived(const RPZGameSession &gameSession);

};