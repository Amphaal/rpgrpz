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

#include "RPZServer.h"

RPZServer::RPZServer() : JSONLogger(QStringLiteral(u"[Server]")) {}

RPZServer::~RPZServer() {
    if (this->_mapHasLoaded) this->_saveSnapshot();
    if(this->_hints) delete this->_hints;
}

void RPZServer::_saveSnapshot() {
    auto savedAt = this->_hints->snapshotSave(AppContext::getServerMapAutosaveFolderLocation());
    this->log(QStringLiteral(u"Map snapshot saved to %1").arg(savedAt));
}

void RPZServer::run() {
    this->_hints = new AtomsStorage(Payload::Interactor::RPZServer);

    // init
    this->log("Starting server...");
    auto result = this->listen(QHostAddress::Any, AppContext::UPNP_DEFAULT_TARGET_PORT.toInt());

    if (!result) {
        qWarning() << "RPZServer : Error while starting to listen >>" << this->errorString();
        emit failed();
        return;
    }

    QObject::connect(
        this, &QTcpServer::newConnection,
        this, &RPZServer::_onNewConnection
    );

    this->log("Succesfully listening");
    emit listening();
}

void RPZServer::_attributeRoleToUser(JSONSocket* socket, RPZUser &associatedUser, const RPZHandshake &handshake) {
    auto isLocalConnection = socket->localAddress().isLoopback();
    auto noHost = !this->_socketsByRole.value(RPZUser::Role::Host).count();

    auto incarnation = handshake.incarnatingAs();
    auto wantsToIncarnate = !incarnation.isEmpty();

    // may elevate as Host
    if (isLocalConnection && noHost && !wantsToIncarnate) {
        associatedUser.setRole(RPZUser::Role::Host);
    } else if (wantsToIncarnate) {  // may elevate as Player
        associatedUser.setCharacter(incarnation);
        associatedUser.setRole(RPZUser::Role::Player);
        associatedUser.randomiseColor();
    }

    // add to roles
    this->_socketsByRole[associatedUser.role()].insert(socket);
}

void RPZServer::incomingConnection(qintptr socketDescriptor) {
    auto socket = new JSONSocket(this, this);
    socket->setSocketDescriptor(socketDescriptor);
    addPendingConnection(socket);
}

int RPZServer::_sendToSockets(const QList<JSONSocket*> toSendTo, const RPZJSON::Method &method, const QVariant &data) {
    auto expected = toSendTo.count();
    int sent = 0;

    for (const auto socket : toSendTo) {
        auto success = socket->sendToSocket(method, data);
        sent += (int)success;
    }

    this->log(method, QStringLiteral(u"sent to [%1/%2] clients").arg(sent).arg(expected));
    return sent;
}

void RPZServer::_onNewConnection() {
    // new connection,store it
    auto clientSocket = dynamic_cast<JSONSocket*>(this->nextPendingConnection());

    // create new user with id
    RPZUser user;
    user.shuffleId();

    // add to internal lists
    auto userId = user.id();
    this->_usersById.insert(userId, user);
    this->_idsByClientSocket.insert(clientSocket, userId);
    this->_clientSocketById.insert(userId, clientSocket);
    this->_playersContexts.insert(clientSocket, {});

    // clear on client disconnect
    QObject::connect(
        clientSocket, &QAbstractSocket::disconnected,
        this, &RPZServer::_onClientSocketDisconnected
    );

    // on data reception
    QObject::connect(
        clientSocket, &JSONSocket::PayloadReceived,
        this, &RPZServer::_onClientPayloadReceived
    );

    // audit
    QObject::connect(
        clientSocket, &JSONSocket::JSONSendingStarted,
        this, &RPZServer::_onSendingToClientStarted
    );
    QObject::connect(
        clientSocket, &JSONSocket::JSONUploading,
        this, &RPZServer::_onUploadingToClient
    );
    QObject::connect(
        clientSocket, &JSONSocket::JSONUploaded,
        this, &RPZServer::_onJSONUploadedToClient
    );
    QObject::connect(
        clientSocket, &JSONSocket::JSONUploadInterrupted,
        this, &RPZServer::_onClientUploadInterrupted
    );

    // signals new connection
    auto newIp = clientSocket->peerAddress().toString();
    this->log(QStringLiteral(u"New connection from %1").arg(newIp));
}

void RPZServer::_onClientUploadInterrupted() {
    emit clientUploadInterrupted();
}

void RPZServer::_onSendingToClientStarted(RPZJSON::Method method, qint64 totalToUpload) {
    auto socket = dynamic_cast<JSONSocket*>(this->sender());
    auto sentTo = this->_getUser(socket);
    emit startUploadToClient(method, totalToUpload, sentTo);
}

void RPZServer::_onUploadingToClient(qint64 bytesUploaded) {
    emit uploadingToClient(bytesUploaded);
}

void RPZServer::_onJSONUploadedToClient() {
    emit uploadedToClient();
}

void RPZServer::_onClientSocketDisconnected() {
    auto disconnectedSocket = dynamic_cast<JSONSocket*>(this->sender());

    // remove socket / user from inner db
    auto idToRemove = this->_idsByClientSocket.take(disconnectedSocket);
    this->_clientSocketById.remove(idToRemove);
    auto removedUser = this->_usersById.take(idToRemove);
    this->_formatedUsernamesByUserId.remove(removedUser.whisperTargetName());

    // remove from dice throw contexts
    this->_playersContexts.remove(disconnectedSocket);

    // desalocate host role
    auto role = removedUser.role();
    this->_socketsByRole[role].remove(disconnectedSocket);
    if (!this->_socketsByRole.count(role)) this->_socketsByRole.remove(role);

    auto disconnectingAddress = disconnectedSocket->peerAddress().toString();
    this->log(QStringLiteral(u"%1 disconnected !").arg(disconnectingAddress));

    disconnectedSocket->deleteLater();

    // tell other clients that the user is gone
    this->_sendToAll(RPZJSON::Method::UserOut, QString::number(idToRemove));  // must be saved as string to prevent JSON parser lack of precision on double conversion
    this->_logUserAsMessage(nullptr, RPZJSON::Method::UserOut, removedUser);
}

void RPZServer::_logUserAsMessage(JSONSocket* userSocket, const RPZJSON::Method &method, const RPZUser &user) {
    // define message
    RPZMessage msg("", method == RPZJSON::Method::UserIn ?
                        MessageInterpreter::Command::C_UserLogIn
                        : MessageInterpreter::Command::C_UserLogOut);
    msg.setOwnership(user);

    // store it
    this->_messages.insert(msg.id(), msg);

    // broadcast
    if (userSocket) this->_sendToAllExcept(userSocket, RPZJSON::Method::Message, msg);
    else
        this->_sendToAll(RPZJSON::Method::Message, msg);
}

void RPZServer::_onClientPayloadReceived(const RPZJSON::Method &method, const QVariant &data) {
    // find target
    auto target = dynamic_cast<JSONSocket*>(this->sender());
    emit isActive();

    switch (method) {
        case RPZJSON::Method::PingHappened: {
            this->_sendToAllExcept(target, method, data);  // notify everyone else
        }
        break;

        case RPZJSON::Method::SharedDocumentRequested: {
            auto requestedHash = data.toString();
            auto document = SharedDocHint::getSharedDocument(requestedHash);

            // notify everyone else
            target->sendToSocket(RPZJSON::Method::SharedDocumentRequested, document);
        }
        break;

        case RPZJSON::Method::SharedDocumentAvailable: {
            // notify everyone else
            this->_sendToAllExcept(target, method, data);
        }
        break;

        case RPZJSON::Method::QuickDrawHappened: {
            this->_sendToAllExcept(target, method, data);  // notify everyone else
        }
        break;

        case RPZJSON::Method::CharacterChanged: {
            // update character of the user
            RPZCharacter character(data.toHash());
            auto &user = this->_getUser(target);
            user.setCharacter(character);

            // notify everyone else
            this->_sendToAllExcept(target, RPZJSON::Method::UserDataChanged, user);
        }
        break;

        case RPZJSON::Method::Message: {
            RPZMessage message(data.toHash());
            auto &user = this->_getUser(target);
            message.setOwnership(user);  // force corresponding user to it then store it
            this->_interpretMessage(target, message);
        }
        break;

        case RPZJSON::Method::AudioStreamUrlChanged: {
            StreamPlayStateTracker tracker(data.toHash());
            this->_tracker = tracker;

            this->_sendToAllExcept(target, method, this->_tracker);
        }
        break;

        case RPZJSON::Method::AudioStreamPlayingStateChanged: {
            auto isPlaying = data.toBool();
            this->_tracker.updatePlayingState(isPlaying);

            this->_sendToAllExcept(target, method, data);
        }
        break;

        case RPZJSON::Method::AudioStreamPositionChanged: {
            auto newPosInMs = data.value<qint64>();
            this->_tracker.updatePositionInMSecs(newPosInMs);

            this->_sendToAllExcept(target, method, data);
        }
        break;

        case RPZJSON::Method::AskForAssets: {
            // cast
            QSet<RPZAsset::Hash> requested;
            for (const auto &var : data.toList()) {
                requested += var.toString();
            }
            auto requestedCount = requested.count();

            // determine available assets to upload
            auto available = AssetsDatabase::get()->getStoredAssetHashes();
            requested.intersect(available);

            // if there are any, tell the client
            if (auto toUploadCount = requested.count()) {
                // log
                auto logMsg = QStringLiteral(u"Assets : %1 / %2 requested asset(s) ready to upload")
                        .arg(toUploadCount)
                        .arg(requestedCount);
                this->log(logMsg);


                // rebundle
                QVariantList remaining;
                for (const auto &i : requested) remaining += i;

                // send
                target->sendToSocket(RPZJSON::Method::AvailableAssetsToUpload, remaining);
            }

            // package each asset and send it to user
            for (const auto &assetHash : requested) {
                auto package = AssetsDatabase::get()->prepareAssetPackage(assetHash);

                if (!package.isPackageCreationSuccessful()) continue;

                target->sendToSocket(RPZJSON::Method::RequestedAsset, package);
            }
        }
        break;

        case RPZJSON::Method::MapChangedHeavily:
        case RPZJSON::Method::MapChanged: {
            auto aPayload = Payloads::autoCast(data.toHash());
            this->_broadcastMapChanges(method, *aPayload, target);
        }
        break;

        case RPZJSON::Method::Handshake: {
            // prepare
            auto &targetUser = this->_getUser(target);
            RPZHandshake handshakePkg(data.toHash());

            // check versions with server, if different, reject
            auto serverVersion = QString(APP_CURRENT_VERSION);
            auto clientVersion = handshakePkg.clientVersion();
            if (clientVersion != serverVersion) {
                target->sendToSocket(RPZJSON::Method::ServerStatus,
                    tr("Your software version is different from the server's one : v%1 (you) / v%2 (server) !")
                    .arg(clientVersion)
                    .arg(serverVersion)
                );
                break;
            }

            // store username
            auto chosenName = handshakePkg.requestedUsername();
            targetUser.setName(chosenName);
            this->_formatedUsernamesByUserId.insert(
                targetUser.whisperTargetName(),
                targetUser.id()
            );

            // determine which role to give to the socket/user
            this->_attributeRoleToUser(target, targetUser, handshakePkg);

            // tell the others that this user exists
            this->_sendToAllExcept(target, RPZJSON::Method::UserIn, targetUser);
            this->_logUserAsMessage(target, RPZJSON::Method::UserIn, targetUser);

            // send game session
            this->_sendGameSession(target, targetUser);
        }
        break;

        default:
            break;
    }

    emit isInactive();
}

//
// HIGH Helpers
//


void RPZServer::_sendGameSession(JSONSocket* toSendTo, const RPZUser &associatedUser) {
    // standard game session
    auto isFullSession = associatedUser.role() != RPZUser::Role::Host;
    auto associatedUserId = associatedUser.id();
    auto sharedDocsNamesStore = SharedDocHint::getNamesStore();

    RPZGameSession gs(
        associatedUserId,
        this->_usersById,
        this->_messages,
        sharedDocsNamesStore,
        isFullSession
    );

    // if requesting full session data...
    if (isFullSession) {
        // stream state
        gs.setStreamState(this->_tracker);

        // map payload
        auto mPayload = this->_hints->generateResetPayload();
        this->_hints->restrictPayload(mPayload);
        gs.setMapPayload(mPayload);
    }

    // send game session
    toSendTo->sendToSocket(RPZJSON::Method::GameSessionSync, gs);
}

void RPZServer::_broadcastMapChanges(const RPZJSON::Method &method, AlterationPayload &payload, JSONSocket * senderSocket) {
    // save for history
    this->_hints->handleAlterationRequest(payload);

    if (payload.type() == Payload::Alteration::Reset) this->_mapHasLoaded = true;

    // add source for outer calls
    auto source = this->_hints->interactorId();
    payload.changeSource(source);

    // might restrict
    if (auto casted = dynamic_cast<AtomRelatedPayload*>(&payload)) {
        // send untouched
        if (this->_socketsByRole.contains(RPZUser::Role::Host)) {
            this->_sendToRoleExcept(senderSocket, RPZUser::Role::Host, method, *casted);
        }

        // send altered
        auto containsPlayers = this->_socketsByRole.contains(RPZUser::Role::Player);
        auto containsObservers = this->_socketsByRole.contains(RPZUser::Role::Observer);
        if (containsPlayers || containsObservers) {
            auto atomsAreLeft = this->_hints->restrictPayload(*casted);

            // if ids are left, send
            if (atomsAreLeft) {
                if (containsPlayers) this->_sendToRoleExcept(senderSocket, RPZUser::Role::Player, method, *casted);
                if (containsObservers) this->_sendToRoleExcept(senderSocket, RPZUser::Role::Observer, method, *casted);
            }
        }
    } else {  // send to registered users but sender...
        this->_sendToAllExcept(senderSocket, method, payload);
    }
}

void RPZServer::_interpretMessage(JSONSocket* sender, RPZMessage &msg) {
    auto msgId = msg.id();
    RPZResponse response;

    switch (const auto cmdType = msg.commandType()) {
        // on unknown command
        case MessageInterpreter::Command::C_Unknown: {
            response = RPZResponse(msgId, RPZResponse::ResponseCode::UnknownCommand);
        }
        break;

        // on help
        case MessageInterpreter::Command::Help: {
            response = RPZResponse(msgId,
                RPZResponse::ResponseCode::HelpManifest,
                MessageInterpreter::help()
            );
        }
        break;

        // on whisper
        case MessageInterpreter::Command::Whisper: {
            // get recipients usernames
            auto textCommand = msg.text();
            auto initialOwner = msg.owner();
            auto recipients = MessageInterpreter::findRecipentsFromText(textCommand);

            // iterate
            QList<QString> notFound;
            for (const auto &recipient : recipients) {
                // find user from recipident
                auto userSocket = this->_getUserSocket(recipient);
                if (!userSocket) {
                    notFound.append(recipient);
                    continue;
                }

                // send to recipient user
                auto textOnly = MessageInterpreter::sanitizeText(textCommand);

                // create a new message
                auto newMessage = RPZMessage(textOnly, MessageInterpreter::Command::Whisper);
                newMessage.setOwnership(initialOwner);

                // send new message
                userSocket->sendToSocket(RPZJSON::Method::Message, newMessage);
            }

            // inform whisperer of any unfound users
            if (notFound.count()) {
                QVariantList usernamesNotFound;
                for (auto &un : notFound) usernamesNotFound.append(un);

                response = RPZResponse(msgId,
                    RPZResponse::ResponseCode::ErrorRecipients,
                    usernamesNotFound
                );
            }
        }
        break;

        // on standard message
        case MessageInterpreter::Command::Say:
        default: {
            //
            // Prepare helper methods
            //
                auto sendAndStore = [&msgId, &msg, &sender, this]() {
                    // store message
                    this->_messages.insert(msgId, msg);

                    // push to all sockets
                    this->_sendToAllExcept(sender, RPZJSON::Method::Message, msg);
                };

                auto reponseAsError = [&response, &msgId](const QString &errStr) {
                    response = RPZResponse(msgId, RPZResponse::ResponseCode::DiceThrowError, errStr);
                };
            //
            //
            //

            // if is dice throw, ask server to interpret before anything else
            if (msg.isDiceThrowCommand()) {
                auto &playerContext = _playersContexts[sender];

                try {
                    // resolve dice throw
                    auto extract = Dicer::Parser::parseThrowCommand(&this->_gameContext, &playerContext, msg.text().toStdString());
                    auto resolved = Dicer::Resolver::resolve(&this->_gameContext, &playerContext, extract);

                    // store send command and commit to others connected peers as it has not thrown
                    sendAndStore();

                    // prepare results message
                    RPZMessage dThrowMsg(QString::fromStdString(resolved.commandAndResultAsString()), MessageInterpreter::Command::C_DiceThrow);
                    dThrowMsg.setOwnership(msg.owner());
                    if(resolved.hasSingleResult()) dThrowMsg.setDiceThrowResult(resolved.singleResult());

                    this->_messages.insert(dThrowMsg.id(), dThrowMsg);      // store results
                    this->_sendToAll(RPZJSON::Method::Message, dThrowMsg);  // send to all
                //
                } catch(const Dicer::DicerException &ex) {
                    reponseAsError(QString(ex.what()));
                //
                } catch(...) {  // default
                    reponseAsError(tr("Cannot interpret throw command !"));
                }

            } else {
                sendAndStore();
            }
        }
        break;
    }

    // set ack if no specific reponse set
    if (!response.answerer()) response = RPZResponse(msgId);

    // send response
    sender->sendToSocket(RPZJSON::Method::ServerResponse, response);
}

//
// LOW Helpers
//

void RPZServer::_sendToAllExcept(JSONSocket* toExclude, const RPZJSON::Method &method, const QVariant &data) {
    auto toSendTo = this->_clientSocketById.values();

    toSendTo.removeOne(toExclude);
    if (!toSendTo.count()) return;

    this->_sendToSockets(toSendTo, method, data);
}

void RPZServer::_sendToRoleExcept(JSONSocket* toExclude, const RPZUser::Role &role, const RPZJSON::Method &method, const QVariant &data) {
    auto toSendTo = this->_socketsByRole.value(role);

    toSendTo.remove(toExclude);
    if (!toSendTo.count()) return;

    this->_sendToSockets(toSendTo.values(), method, data);
}

void RPZServer::_sendToAll(const RPZJSON::Method &method, const QVariant &data) {
    this->_sendToSockets(this->_clientSocketById.values(), method, data);
}

JSONSocket* RPZServer::_getUserSocket(const QString &formatedUsername) {
    auto id = this->_formatedUsernamesByUserId.value(formatedUsername);
    return this->_clientSocketById.value(id);
}

RPZUser& RPZServer::_getUser(JSONSocket* socket) {
    auto id = this->_idsByClientSocket.value(socket);
    return this->_usersById[id];
}
