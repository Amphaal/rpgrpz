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
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "src/ui/chat/logs/_base/LogContainer.h"
#include "src/ui/chat/logs/components/LogContent.hpp"
#include "src/ui/chat/logs/components/LogText.hpp"

#include "src/shared/models/messaging/RPZMessage.h"
#include "src/shared/models/messaging/RPZResponse.h"

#include "src/ui/_others/ConnectivityObserver.h"

#include <QBoxLayout>

class MessagesLog : public LogContainer, public ConnectivityObserver {
    
    Q_OBJECT
    
    public:
        MessagesLog(QWidget *parent = nullptr);

        void handleResponse(const RPZResponse &response);
        
        void handleLocalMessage(RPZMessage &msg);
        void handleRemoteMessage(const RPZMessage &msg);
        void handleHistoryMessage(const RPZMessage &msg);

    signals:
        void notificationCountUpdated(int newCount);

    private:
        QList<Stampable::Id> _msgIdsNotSeen;

        void _handleMessage(const RPZMessage &msg, bool isLocal = false, bool fromHistory = false);
        void changeEvent(QEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
};