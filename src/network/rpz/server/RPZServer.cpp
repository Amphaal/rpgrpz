#include "RPZServer.h" 

RPZServer::RPZServer() : JSONLogger(QStringLiteral(u"[Server]")) {

    //connect to new connections (proxy through windowed function to ensure event is handled into the server thread)
    QObject::connect(
        this, &QTcpServer::newConnection, 
        this, &RPZServer::_onNewConnection
    );

};

RPZServer::~RPZServer() {
    if(this->_hostSocket) delete this->_hostSocket;
    if(this->_hints) delete this->_hints;
}

void RPZServer::run() { 

    //init
    this->_hints = new AtomsStorage(Payload::Source::RPZServer);

    this->log("Starting server...");

    auto result = this->listen(QHostAddress::Any, AppContext::UPNP_DEFAULT_TARGET_PORT.toInt());

    if(!result) {
        qWarning() << "RPZServer : Error while starting to listen >>" << this->errorString();
        emit error();
        emit stopped();
    } else {
        this->log("Succesfully listening");
        emit listening();
    }

};

void RPZServer::_attributeRoleToUser(JSONSocket* socket, RPZUser &associatedUser, const RPZHandshake &handshake) {
    
    auto isLocalConnection = socket->socket()->localAddress().isLoopback();
    auto noHost = !this->_hostSocket;

    auto incarnation = handshake.incarnatingAs();
    auto wantsToIncarnate = !incarnation.isEmpty();
    
    //may elevate as Host
    if(isLocalConnection && noHost && !wantsToIncarnate) {
        this->_hostSocket = socket;
        associatedUser.setRole(RPZUser::Role::Host);
    } 
    
    //may elevate as Player
    else if (wantsToIncarnate) {
        associatedUser.setCharacter(incarnation);
        associatedUser.setRole(RPZUser::Role::Player);
    }

}

void RPZServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = new JSONSocket(this, this, this->nextPendingConnection());
        
        //create new user with id
        RPZUser user;
        user.shuffleId();
        user.randomiseColor();

        //add to internal lists
        auto userId = user.id();
        this->_usersById.insert(userId, user);
        this->_idsByClientSocket.insert(clientSocket, userId);
        this->_clientSocketById.insert(userId, clientSocket);
        
        //clear on client disconnect
        QObject::connect(
            clientSocket->socket(), &QAbstractSocket::disconnected,
            [&, clientSocket]() {
				this->_onClientSocketDisconnected(clientSocket);
			}
        );

        //on data reception
        QObject::connect(
            clientSocket, &JSONSocket::JSONReceived,
            this, &RPZServer::_routeIncomingJSON
        );

        //signals new connection
        auto newIp = clientSocket->socket()->peerAddress().toString();
        this->log(QStringLiteral(u"New connection from %1").arg(newIp));

}

void RPZServer::_onClientSocketDisconnected(JSONSocket* disconnectedSocket) {

    //remove socket / user from inner db
    auto idToRemove = this->_idsByClientSocket.take(disconnectedSocket);
    this->_clientSocketById.remove(idToRemove);
    auto removedUser = this->_usersById.take(idToRemove);
    this->_formatedUsernamesByUserId.remove(removedUser.whisperTargetName());

    //desalocate host
    if(this->_hostSocket == disconnectedSocket) {
        this->_hostSocket = nullptr;
    }

    auto disconnectingAddress = disconnectedSocket->socket()->peerAddress().toString();
    this->log(QStringLiteral(u"%1 disconnected !").arg(disconnectingAddress));

    delete disconnectedSocket;

    //tell other clients that the user is gone
    this->_sendToAll(RPZJSON::Method::UserOut, idToRemove);

}

void RPZServer::_routeIncomingJSON(JSONSocket* target, const RPZJSON::Method &method, const QVariant &data) {

    QMetaObject::invokeMethod(ProgressTracker::get(), "serverIsActive");

    switch(method) {
        
        case RPZJSON::Method::CharacterChanged: {
            
            //update character of the user
            RPZCharacter character(data.toHash());
            auto &user = this->_getUser(target);
            user.setCharacter(character);

            //notify everyone else
            this->_sendToAllButSelf(target, RPZJSON::Method::UserDataChanged, user);

        }
        break;

        case RPZJSON::Method::Message: {
            RPZMessage message(data.toHash());
            auto &user = this->_getUser(target);
            message.setOwnership(user); //force corresponding user to it then store it
            this->_interpretMessage(target, message);
        }
        break;

        case RPZJSON::Method::AudioStreamUrlChanged: {
            
            StreamPlayStateTracker tracker(data.toHash());
            this->_tracker = tracker;

            this->_sendToAllButSelf(target, RPZJSON::Method::AudioStreamUrlChanged, this->_tracker);

        }
        break;

        case RPZJSON::Method::AudioStreamPlayingStateChanged: {
            
            auto isPlaying = data.toBool();
            this->_tracker.updatePlayingState(isPlaying);
            
            this->_sendToAllButSelf(target, RPZJSON::Method::AudioStreamPlayingStateChanged, data);

        }
        break;

        case RPZJSON::Method::AudioStreamPositionChanged: {
            
            auto newPosInMs = data.value<qint64>();
            this->_tracker.updatePositionInMSecs(newPosInMs);

            this->_sendToAllButSelf(target, RPZJSON::Method::AudioStreamPositionChanged, data);

        }
        break;

        case RPZJSON::Method::AskForAssets: {   

            //cast
            QSet<RPZAsset::Hash> requested;
            for(auto &var : data.toList()) {
                requested += var.toString();
            }
            auto requestedCount = requested.count();

            //determine available assets to upload
            auto available = AssetsDatabase::get()->getStoredAssetHashes();
            requested.intersect(available);

            //if there are any, tell the client
            if(auto toUploadCount = requested.count()) {

                //log             
                auto logMsg = QStringLiteral(u"Assets : %1 / %2 requested asset(s) ready to upload")
                        .arg(toUploadCount)
                        .arg(requestedCount);
                this->log(logMsg);
                
                
                //rebundle
                QVariantList remaining;
                for(auto &i : requested) remaining += i;

                //send
                target->sendToSocket(RPZJSON::Method::AvailableAssetsToUpload, remaining);

            }

            //package each asset and send it to user
            for(auto &assetHash : requested) {

                auto package = AssetsDatabase::get()->prepareAssetPackage(assetHash);

                if(!package.isPackageCreationSuccessful()) continue;

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
            
            //prepare
            auto &targetUser = this->_getUser(target);
            RPZHandshake handshakePkg(data.toHash());
            
            //check versions with server, if different, reject
            auto serverVersion = QString(APP_CURRENT_VERSION);
            auto clientVersion = handshakePkg.clientVersion();
            if(clientVersion != serverVersion) {
                target->sendToSocket(RPZJSON::Method::ServerStatus, 
                    tr("Your software version is different from the server's one : v%1 (you) / v%2 (server) !")
                    .arg(clientVersion)
                    .arg(serverVersion)
                );
                break;
            }

            //store username
            auto chosenName = handshakePkg.requestedUsername();
            targetUser.setName(chosenName);
            this->_formatedUsernamesByUserId.insert(
                targetUser.whisperTargetName(), 
                targetUser.id()
            );
            
            //determine which role to give to the socket/user
            this->_attributeRoleToUser(target, targetUser, handshakePkg);

            //distribute user ack events to clients
            this->_newUserAcknoledged(target, targetUser);

            //send history to the client
            this->_sendStoredMessages(target);
            
            //if is not host
            if(target != this->_hostSocket)  {
                                
                //send data played stream informations
                this->_sendPlayedStream(target); 

                //send stored history
                this->_sendMapHistory(target);

            }

        }
        break;

        default:
            break;
        
    }

    QMetaObject::invokeMethod(ProgressTracker::get(), "serverIsInactive");

}

//
// HIGH Helpers
//

void RPZServer::_newUserAcknoledged(JSONSocket* socket, const RPZUser &userToAck) {
    
    //tell associated user's socket his identity
    socket->sendToSocket(RPZJSON::Method::AckIdentity, userToAck);

    //send whole users database to socket
    auto method = RPZJSON::Method::AllConnectedUsers;
    auto toSend = this->_usersById.toVList();
    this->_sendToAll(method, toSend);

    //log
    this->log(method, QStringLiteral(u"Now %1 clients logged").arg(toSend.count()));

    //tell the others that this user exists
    this->_sendToAllButSelf(socket, RPZJSON::Method::UserIn, userToAck);

}

void RPZServer::_sendStoredMessages(JSONSocket * clientSocket) {
    
    //send messages...
    auto messagesToSend = this->_messages.toVList();
    auto method = RPZJSON::Method::ChatLogHistory;
    clientSocket->sendToSocket(method, messagesToSend);

    //log
    auto countMsgs = this->_messages.count();
    auto logMsg = QStringLiteral(u"%1 stored messages sent to \"%2\"")
                        .arg(countMsgs)
                        .arg(clientSocket->socket()->peerAddress().toString());
    this->log(method, logMsg);
}

void RPZServer::_broadcastMapChanges(RPZJSON::Method method, AlterationPayload &payload, JSONSocket * senderSocket) {

    //prevent alteration ack if sender is not host
    if(this->_getUser(senderSocket).role() != RPZUser::Role::Host) return;

    //save for history
    this->_hints->handleAlterationRequest(payload);

    //add source for outer calls
    auto source = this->_hints->source();
    payload.changeSource(source);

    //send to registered users but sender...
    this->_sendToAllButSelf(senderSocket, method, payload);
}

void RPZServer::_sendMapHistory(JSONSocket * clientSocket) {

    //generate payload
    auto payload = this->_hints->generateResetPayload();

    //send it
    clientSocket->sendToSocket(RPZJSON::Method::MapChangedHeavily, payload);

}

void RPZServer::_interpretMessage(JSONSocket* sender, RPZMessage &msg){
    
    auto msgId = msg.id();
    RPZResponse response;

    switch(msg.commandType()) {
        
        //on unknown command
        case MessageInterpreter::Command::C_Unknown: {
            response = RPZResponse(msgId, RPZResponse::ResponseCode::UnknownCommand);
        }
        break;

        //on help
        case MessageInterpreter::Command::Help: {
            response = RPZResponse(msgId, 
            RPZResponse::ResponseCode::HelpManifest, 
            MessageInterpreter::help()
            );
        }
        break;

        //on whisper
        case MessageInterpreter::Command::Whisper: {
            
            //get recipients usernames
            auto textCommand = msg.text();
            auto initialOwner = msg.owner();
            auto recipients = MessageInterpreter::findRecipentsFromText(textCommand);

            //iterate
            QList<QString> notFound;
            for(auto &recipient : recipients) {
                
                //find user from recipident
                auto userSocket = this->_getUserSocket(recipient);
                if(!userSocket) {
                    notFound.append(recipient);
                    continue;
                }

                //send to recipient user
                auto textOnly = MessageInterpreter::sanitizeText(textCommand);
                
                //create a new message 
                auto newMessage = RPZMessage(textOnly, MessageInterpreter::Command::Whisper);
                newMessage.setOwnership(initialOwner);

                //send new message
                userSocket->sendToSocket(RPZJSON::Method::Message, newMessage);
            }

            //inform whisperer of any unfound users
            if(notFound.count()) {

                QVariantList usernamesNotFound;
                for( auto &un : notFound) usernamesNotFound.append(un);

                response = RPZResponse(msgId, 
                    RPZResponse::ResponseCode::ErrorRecipients, 
                    usernamesNotFound
                );
                
            }
            
        }
        break;

        //on standard message
        case MessageInterpreter::Command::Say:
        default: {
            
            //store message
            this->_messages.insert(msgId, msg);

            //push to all sockets
            this->_sendToAllButSelf(sender, RPZJSON::Method::Message, msg);

            //send dices throws if requested
            this->_maySendAndStoreDiceThrows(msg.text());

        }
        break;
    }

    //set ack if no specific reponse set
    if(!response.answerer()) response = RPZResponse(msgId);

    //send response
    sender->sendToSocket(RPZJSON::Method::ServerResponse, response);
}

void RPZServer::_maySendAndStoreDiceThrows(const QString &text) {
    
    //check if throws are requested
    auto throws = MessageInterpreter::findDiceThrowsFromText(text);
    if(throws.isEmpty()) return;

    //generate values
    MessageInterpreter::generateValuesOnDiceThrows(throws);

    //create message parts
    QList<QString> throwsMsgList;
    for(auto &dThrow : throws) {
        
        //sub list of values
        QList<QString> sub;
        for(auto &pair : dThrow.pairedValues) {
            sub += QStringLiteral(u"%1%2")
                        .arg(StringHelper::toSuperScript(pair.second))
                        .arg(pair.first);
        }
        auto subJoin = sub.join(", ");

        //join values
        auto joined = QStringLiteral(u"%1 : {%2}").arg(dThrow.name).arg(subJoin);

        //display avg if multiple values
        if(sub.count() > 1) joined += QStringLiteral(u" x̄ ") + QString::number(dThrow.avg, 'f', 2);

        //add to topmost list
        throwsMsgList += joined;

    }

    //append it
    auto msg = throwsMsgList.join(", ");
    RPZMessage dThrowMsg(msg, MessageInterpreter::Command::C_DiceThrow);

    //store message
    this->_messages.insert(dThrowMsg.id(), dThrowMsg);

    //send to all
    this->_sendToAll(RPZJSON::Method::Message, dThrowMsg);

}

//
//
//

//
// LOW Helpers
//

void RPZServer::_sendToAllButSelf(JSONSocket* toExclude, const RPZJSON::Method &method, const QVariant &data) {
    JSONSocket::sendToSockets(this, this->_clientSocketById.values(), method, data, toExclude);
}

void RPZServer::_sendToAll(const RPZJSON::Method &method, const QVariant &data) {
    JSONSocket::sendToSockets(this, this->_clientSocketById.values(), method, data);
}

JSONSocket* RPZServer::_getUserSocket(const QString &formatedUsername) {
    auto id = this->_formatedUsernamesByUserId.value(formatedUsername);
    return this->_clientSocketById.value(id);
}

RPZUser& RPZServer::_getUser(JSONSocket* socket) {
    auto id = this->_idsByClientSocket.value(socket);
    return this->_usersById[id];
}

void RPZServer::_sendPlayedStream(JSONSocket* socket) {
    if(!this->_tracker.isSomethingPlaying()) return;   
    socket->sendToSocket(RPZJSON::Method::AudioStreamUrlChanged, this->_tracker);
}