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

#include "src/network/rpz/_any/JSONSocket.h"

#include "src/shared/hints/AtomsStorage.h"
#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/messaging/RPZMessage.h"
#include "src/shared/models/RPZUser.h"
#include "src/shared/models/messaging/RPZResponse.h"
#include "src/shared/models/network/RPZHandshake.h"
#include "src/shared/models/network/RPZGameSession.hpp"
#include "src/shared/models/RPZQuickDrawBits.hpp"

#include "src/shared/database/AssetsDatabase.h"

#include "src/helpers/_appContext.h"
#include "src/helpers/StringHelper.hpp"

#include "src/shared/commands/MessageInterpreter.h"
#include "src/shared/audio/StreamPlayStateTracker.hpp"

#include "src/shared/models/RPZSharedDocument.hpp"

#include "src/shared/hints/SharedDocHint.hpp"

#include "src/network/rpz/_any/JSONLogger.hpp"

#include <dicer/Contexts.hpp>
#include <dicer/Resolver.hpp>
#include <dicer/Parser.hpp>

class RPZServer : public QTcpServer, public JSONLogger {
    Q_OBJECT

 public:
    RPZServer();
    ~RPZServer();

    void run();

 signals:
    void listening();
    void failed();

    void isActive();
    void isInactive();

    void startUploadToClient(RPZJSON::Method method, qint64 totalToUpload, const RPZUser &sentTo);
    void uploadingToClient(qint64 bytesUploaded);
    void uploadedToClient();
    void clientUploadInterrupted();

 protected:
    void incomingConnection(qintptr socketDescriptor) override;

 private slots:
    void _onClientSocketDisconnected();
    void _onClientPayloadReceived(const RPZJSON::Method &method, const QVariant &data);

    void _onSendingToClientStarted(RPZJSON::Method method, qint64 totalToUpload);
    void _onUploadingToClient(qint64 bytesUploaded);
    void _onJSONUploadedToClient();
    void _onClientUploadInterrupted();

 private:
    bool _mapHasLoaded = false;

    void _saveSnapshot();

    RPZMap<RPZUser> _usersById;
    QHash<JSONSocket*, RPZUser::Id> _idsByClientSocket;
    QHash<RPZUser::Id, JSONSocket*> _clientSocketById;
    QHash<QString, RPZUser::Id> _formatedUsernamesByUserId;

    QHash<RPZUser::Role, QSet<JSONSocket*>> _socketsByRole;

    // music
    StreamPlayStateTracker _tracker;

    // users
    RPZUser& _getUser(JSONSocket* socket);
    JSONSocket* _getUserSocket(const QString &formatedUsername);
    void _attributeRoleToUser(JSONSocket* socket, RPZUser &associatedUser, const RPZHandshake &handshake);

    // map atoms
    AtomsStorage* _hints = nullptr;
    void _broadcastMapChanges(const RPZJSON::Method &method, AlterationPayload &payload, JSONSocket * senderSocket);

    // game session
    void _sendGameSession(JSONSocket* toSendTo, const RPZUser &associatedUser);

    // dices throws
    Dicer::GameContext _gameContext;
    QHash<JSONSocket*, Dicer::PlayerContext> _playersContexts;

    // messages
    RPZMap<RPZMessage> _messages;
    void _interpretMessage(JSONSocket* sender, RPZMessage &msg);
    void _logUserAsMessage(JSONSocket* userSocket, const RPZJSON::Method &method, const RPZUser &user);

    // internal
    void _onNewConnection();

    // sending helpers
    void _sendToAll(const RPZJSON::Method &method, const QVariant &data);
    void _sendToAllExcept(JSONSocket* toExclude, const RPZJSON::Method &method, const QVariant &data);
    void _sendToRoleExcept(JSONSocket* toExclude, const RPZUser::Role &role, const RPZJSON::Method &method, const QVariant &data);
    int _sendToSockets(
        const QList<JSONSocket*> toSendTo,
        const RPZJSON::Method &method,
        const QVariant &data
    );
};
