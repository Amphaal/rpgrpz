#include "RPZServer.h" 

RPZServer::RPZServer() {

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
    this->_hints = new AtomsStorage(AlterationPayload::Source::RPZServer);

    qDebug() << "RPZServer : Starting server...";

    auto result = this->listen(QHostAddress::Any, AppContext::UPNP_DEFAULT_TARGET_PORT.toInt());

    if(!result) {
        qWarning() << "RPZServer : Error while starting to listen >>" << this->errorString();
        emit error();
        emit stopped();
    } else {
        qDebug() << "RPZServer : Succesfully listening !";
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
        auto clientSocket = new JSONSocket(this, "RPZServer", this->nextPendingConnection());
        
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
        qDebug() << "RPZServer : New connection from" << newIp;

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
    qDebug() << "RPZServer : " << disconnectingAddress << " disconnected !";

    delete disconnectedSocket;

    //tell other clients that the user is gone
    this->_sendToAll(JSONMethod::UserOut, idToRemove);

}

void RPZServer::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {

    QMetaObject::invokeMethod(ProgressTracker::get(), "serverIsActive");

    switch(method) {
        
        case JSONMethod::CharacterChanged: {
            
            //update character of the user
            RPZCharacter character(data.toHash());
            auto &user = this->_getUser(target);
            user.setCharacter(character);

            //notify everyone else
            this->_sendToAllButSelf(target, JSONMethod::UserDataChanged, user);

        }
        break;

        case JSONMethod::Message: {
            RPZMessage message(data.toHash());
            auto &user = this->_getUser(target);
            message.setOwnership(user); //force corresponding user to it then store it
            this->_interpretMessage(target, message);
        }
        break;

        case JSONMethod::AudioStreamUrlChanged: {
            
            StreamPlayStateTracker tracker(data.toHash());
            this->_tracker = tracker;

            this->_sendToAllButSelf(target, JSONMethod::AudioStreamUrlChanged, this->_tracker);

        }
        break;

        case JSONMethod::AudioStreamPlayingStateChanged: {
            
            auto isPlaying = data.toBool();
            this->_tracker.updatePlayingState(isPlaying);
            
            this->_sendToAllButSelf(target, JSONMethod::AudioStreamPlayingStateChanged, data);

        }
        break;

        case JSONMethod::AudioStreamPositionChanged: {
            
            auto newPosInMs = data.value<qint64>();
            this->_tracker.updatePositionInMSecs(newPosInMs);

            this->_sendToAllButSelf(target, JSONMethod::AudioStreamPositionChanged, data);

        }
        break;

        case JSONMethod::AskForAssets: {   

            //cast
            QSet<RPZAssetHash> requested;
            for(auto &var : data.toList()) {
                requested += var.toString();
            }
            auto requestedCount = requested.count();

            //determine available assets to upload
            auto available = AssetsDatabase::get()->getStoredAssetsIds();
            requested.intersect(available);

            //if there are any, tell the client
            if(auto toUploadCount = requested.count()) {

                //log
                qDebug() << QString("Assets : %1 / %2 requested asset(s) ready to upload")
                                            .arg(toUploadCount)
                                            .arg(requestedCount)
                                            .toStdString().c_str();
                
                //rebundle
                QVariantList remaining;
                for(auto &i : requested) remaining += i;

                //send
                target->sendJSON(JSONMethod::AvailableAssetsToUpload, remaining);

            }

            //package each asset and send it to user
            for(auto &assetId : requested) {
                auto package = AssetsDatabase::get()->prepareAssetPackage(assetId);
                target->sendJSON(JSONMethod::RequestedAsset, package);
            }

        }
        break;

        case JSONMethod::MapChangedHeavily:
        case JSONMethod::MapChanged: {
            auto aPayload = Payloads::autoCast(data.toHash());
            this->_broadcastMapChanges(method, *aPayload, target);
        }
        break;

        case JSONMethod::Handshake: {   
            
            //prepare
            auto &targetUser = this->_getUser(target);
            RPZHandshake handshakePkg(data.toHash());
            
            //check versions with server, if different, reject
            auto serverVersion = QString(APP_CURRENT_VERSION);
            auto clientVersion = handshakePkg.clientVersion();
            if(clientVersion != serverVersion) {
                target->sendJSON(JSONMethod::ServerStatus, 
                    tr("Versions between host and client are different : v%1 (client) / v%2 (host) !")
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
    socket->sendJSON(JSONMethod::AckIdentity, userToAck);

    //send whole users database to socket
    auto method = JSONMethod::AllConnectedUsers;
    auto toSend = this->_usersById.toVList();
    this->_sendToAll(method, toSend);
    auto msgLog = QString("Now %1 clients logged").arg(toSend.count());
    JSONSocket::_debugLog("RPZServer", method, msgLog);

    //tell the others that this user exists
    this->_sendToAllButSelf(socket, JSONMethod::UserIn, userToAck);

}

void RPZServer::_sendStoredMessages(JSONSocket * clientSocket) {
    
    //send messages...
    auto messagesToSend = this->_messages.toVList();
    auto method = JSONMethod::ChatLogHistory;
    clientSocket->sendJSON(method, messagesToSend);

    //log
    auto countMsgs = this->_messages.count();
    auto logMsg = QString("%1 stored messages sent to \"%2\"")
                        .arg(countMsgs)
                        .arg(clientSocket->socket()->peerAddress().toString());
    JSONSocket::_debugLog("RPZServer", method, logMsg);
}


void RPZServer::_alterIncomingPayloadWithUpdatedOwners(AtomsWielderPayload &wPayload, JSONSocket * senderSocket) {
    
    auto &defaultOwner = this->_getUser(senderSocket); 
    auto updated = wPayload.updateEmptyUser(defaultOwner);
    
    //if sender sent no user atoms
    if(updated.count()) {

        //tell him that he owns them now
        OwnerChangedPayload OCPayload(updated, defaultOwner);
        auto source = this->_hints->source();
        OCPayload.changeSource(source);

        //send...
        this->_sendToAll(JSONMethod::MapChanged, OCPayload);

    }
}

void RPZServer::_broadcastMapChanges(JSONMethod method, AlterationPayload &payload, JSONSocket * senderSocket) {

    if(auto wPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        this->_alterIncomingPayloadWithUpdatedOwners(*wPayload, senderSocket);
    }

    //save for history
    this->_hints->handleAlterationRequest(payload);

    //add source for outer calls
    auto source = this->_hints->source();
    payload.changeSource(source);

    //send to registered users but sender...
    this->_sendToAllButSelf(senderSocket, method, payload);
}

void RPZServer::_sendMapHistory(JSONSocket * clientSocket) {

    //fetch payload
    auto payload = this->_hints->createStatePayload();

    //send it
    clientSocket->sendJSON(JSONMethod::MapChangedHeavily, payload);

}

void RPZServer::_interpretMessage(JSONSocket* sender, RPZMessage &msg){
    
    auto msgId = msg.id();
    RPZResponse response;

    switch(msg.commandType()) {
        
        //on unknown command
        case MessageInterpreter::C_Unknown: {
            response = RPZResponse(msgId, RPZResponse::UnknownCommand);
        }
        break;

        //on help
        case MessageInterpreter::Help: {
            response = RPZResponse(msgId, RPZResponse::HelpManifest, MessageInterpreter::help());
        }
        break;

        //on whisper
        case MessageInterpreter::Whisper: {
            
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
                auto newMessage = RPZMessage(textOnly, MessageInterpreter::Whisper);
                newMessage.setOwnership(initialOwner);

                //send new message
                userSocket->sendJSON(JSONMethod::Message, newMessage);
            }

            //inform whisperer of any unfound users
            if(notFound.count()) {

                QVariantList usernamesNotFound;
                for( auto &un : notFound) usernamesNotFound.append(un);

                response = RPZResponse(msgId, RPZResponse::ErrorRecipients, usernamesNotFound);
                
            }
            
        }
        break;

        //on standard message
        case MessageInterpreter::Say:
        default: {
            
            //store message
            this->_messages.insert(msgId, msg);

            //push to all sockets
            this->_sendToAllButSelf(sender, JSONMethod::Message, msg);

            //send dices throws if requested
            this->_maySendAndStoreDiceThrows(msg.text());

        }
        break;
    }

    //set ack if no specific reponse set
    if(!response.answerer()) response = RPZResponse(msgId);

    //send response
    sender->sendJSON(JSONMethod::ServerResponse, response);
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
            sub += QString("%1%2")
                        .arg(StringHelper::toSuperScript(pair.second))
                        .arg(pair.first);
        }
        QString subJoin = sub.join(", ");

        //join values
        auto joined = QString("%1 : {%2}").arg(dThrow.name).arg(subJoin);

        //display avg if multiple values
        if(sub.count() > 1) joined += QString(" x̄ ") + QString::number(dThrow.avg, 'f', 2);

        //add to topmost list
        throwsMsgList += joined;

    }

    //append it
    auto msg = throwsMsgList.join(", ");
    RPZMessage dThrowMsg(msg, MessageInterpreter::Command::C_DiceThrow);

    //store message
    this->_messages.insert(dThrowMsg.id(), dThrowMsg);

    //send to all
    this->_sendToAll(JSONMethod::Message, dThrowMsg);

}

//
//
//

//
// LOW Helpers
//

void RPZServer::_sendToAllButSelf(JSONSocket* toExclude, const JSONMethod &method, const QVariant &data) {
    for(auto socket : this->_clientSocketById) {
        
        //prevent self send
        if(socket == toExclude) {
            continue;
        } 

        //send to others
        socket->sendJSON(method, data);
    }
}

void RPZServer::_sendToAll(const JSONMethod &method, const QVariant &data) {
    for(auto socket : this->_clientSocketById) {
        socket->sendJSON(method, data);
    }
}

JSONSocket* RPZServer::_getUserSocket(const QString &formatedUsername) {
    auto id = this->_formatedUsernamesByUserId.value(formatedUsername);
    return this->_clientSocketById[id];
}

RPZUser& RPZServer::_getUser(JSONSocket* socket) {
    auto id = this->_idsByClientSocket.value(socket);
    return this->_usersById[id];
}

void RPZServer::_sendPlayedStream(JSONSocket* socket) {
    if(!this->_tracker.isSomethingPlaying()) return;   
    socket->sendJSON(JSONMethod::AudioStreamUrlChanged, this->_tracker);
}