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

#include <QDebug>
#include <QTcpServer>
#include <QHostAddress>
#include <QVector>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QHash>
#include <QVector>

#include "src/network/rpz/_any/JSONSocket.h"

#include "src/shared/hints/AtomsStorage.h"
#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/messaging/RPZMessage.h"
#include "src/shared/models/RPZUser.h"
#include "src/shared/models/messaging/RPZResponse.h"
#include "src/shared/models/network/RPZHandshake.h"
#include "src/shared/models/network/RPZGameSession.hpp"

#include "src/shared/database/AssetsDatabase.h"

#include "src/helpers/_appContext.h"
#include "src/helpers/StringHelper.hpp"

#include "src/shared/commands/MessageInterpreter.h"
#include "src/shared/audio/StreamPlayStateTracker.hpp"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/helpers/StringHelper.hpp"

#include "src/network/rpz/_any/JSONLogger.hpp"

class RPZServer : public QObject, public JSONLogger { 
    
    Q_OBJECT

    public:
        RPZServer();
        ~RPZServer();
    
    public slots:
        void run();

    private slots:
        void _saveSnapshot();

    signals:
        void listening();
        void error();
        void stopped();

    private:
        bool _mapHasLoaded = false;
        QTcpServer* _server = nullptr; 

        RPZMap<RPZUser> _usersById;
        QHash<JSONSocket*, RPZUser::Id> _idsByClientSocket;
        QHash<RPZUser::Id, JSONSocket*> _clientSocketById;
        QHash<QString, RPZUser::Id> _formatedUsernamesByUserId;
        
        QHash<RPZUser::Role, QSet<JSONSocket*>> _socketsByRole;
        
        //music
        StreamPlayStateTracker _tracker;

        //users
        RPZUser& _getUser(JSONSocket* socket);
        JSONSocket* _getUserSocket(const QString &formatedUsername);
        void _attributeRoleToUser(JSONSocket* socket, RPZUser &associatedUser, const RPZHandshake &handshake);

        //map atoms
        AtomsStorage* _hints = nullptr;
        void _broadcastMapChanges(const RPZJSON::Method &method, AlterationPayload &payload, JSONSocket * senderSocket);
        
        //game session
        void _sendGameSession(JSONSocket* toSendTo, const RPZUser &associatedUser);

        //messages
        RPZMap<RPZMessage> _messages;
        void _interpretMessage(JSONSocket* sender, RPZMessage &msg);
        void _maySendAndStoreDiceThrows(const RPZMessage &msg);
        void _logUserAsMessage(JSONSocket* userSocket, const RPZJSON::Method &method, const RPZUser &user);

        //internal
        void _onNewConnection();
        void _onClientSocketDisconnected(JSONSocket* disconnectedSocket);
        void _routeIncomingJSON(JSONSocket* target, const RPZJSON::Method &method, const QVariant &data);
        
        void _sendToAll(const RPZJSON::Method &method, const QVariant &data);
        void _sendToAllExcept(JSONSocket* toExclude, const RPZJSON::Method &method, const QVariant &data);
        void _sendToRoleExcept(JSONSocket* toExclude, const RPZUser::Role &role, const RPZJSON::Method &method, const QVariant &data);

};