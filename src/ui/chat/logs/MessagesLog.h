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

#include "src/ui/chat/logs/_base/LogContainer.h"

#include "src/shared/models/messaging/RPZMessage.h"
#include "src/shared/models/messaging/RPZResponse.h"

#include <QBoxLayout>

class MessagesLog : public LogContainer {
    public:
        MessagesLog(QWidget *parent = nullptr);

        void handleResponse(const RPZResponse &response);
        
        void handleLocalMessage(const RPZMessage &msg);
        void handleNonLocalMessage(const RPZMessage &msg);
    
    private:
        void _handleMessage(const RPZMessage &msg, bool isLocal = false);

        void changeEvent(QEvent *event) override;
};