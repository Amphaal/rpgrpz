#include "RPZServerThread.h" 

RPZServerThread::RPZServerThread(QObject* parent) : QThread(parent) {
    qRegisterMetaType<JSONMethod>("JSONMethod");
};

void RPZServerThread::run() { 

    //init
    auto server = new QTcpServer;  
    this->_hints = new AtomsStorage(AlterationPayload::Source::RPZServer, false);

    qDebug() << "RPZServerThread : Starting server...";

    auto result = server->listen(QHostAddress::Any, AppContext::UPNP_DEFAULT_TARGET_PORT.toInt());

    if(!result) {
        qWarning() << "RPZServerThread : Error while starting to listen >> " + server->errorString();
        emit error();
        return;
    } else {
        qDebug() << "RPZServerThread : Succesfully listening !";
        emit listening();
    }

    //connect to new connections (proxy through windowed function to ensure event is handled into the server thread)
    QObject::connect(
        server, &QTcpServer::newConnection, 
        [=]() { this->_onNewConnection(server); }
    );

    //running event loop
    this->exec();

    //ending server
    server->close();
    qDebug() << "RPZServerThread : Server ended !";

};

void RPZServerThread::_onNewConnection(QTcpServer * server) {
        
        //new connection,store it
        auto clientSocket = new JSONSocket("RPZServerThread", server->nextPendingConnection());
        
        //create new user
        RPZUser user(clientSocket);

        //check if connecting from localhost and no host has already been elected
        if(clientSocket->socket()->localAddress().isLoopback() && !this->_hostSocket) {
            this->_hostSocket = clientSocket;
            user.setRole(RPZUser::Role::Host);
        }

        //add to internal lists
        this->_usersById.insert(user.id(), user);
        this->_idsByClientSocket.insert(clientSocket, user.id());
        
        //clear on client disconnect
        QObject::connect(
            clientSocket->socket(), &QAbstractSocket::disconnected,
            [&, clientSocket]() {
				this->_onDisconnect(clientSocket);
			}
        );

        //on data reception
        QObject::connect(
            clientSocket, &JSONSocket::JSONReceived,
            [&](JSONSocket *target, const JSONMethod &method, const QVariant &data) {
                this->_routeIncomingJSON(target, method, data);
            }
        );

        //signals new connection
        auto newIp = clientSocket->socket()->peerAddress().toString();
        qDebug() << "RPZServerThread : New connection from" << newIp;

}

void RPZServerThread::_onDisconnect(JSONSocket* disconnecting) {

    //remove socket
    const auto idToRemove = this->_idsByClientSocket.take(disconnecting);
    this->_usersById.remove(idToRemove);

    //desalocate host
    if(this->_hostSocket == disconnecting) {
        this->_hostSocket = nullptr;
    }

    auto disconnectingAddress = disconnecting->socket()->peerAddress().toString();
    qDebug() << "RPZServerThread : " << disconnectingAddress << " disconnected !";

    disconnecting->deleteLater();

    //tell other clients that the user is gone
    this->_broadcastUsers();

}

void RPZServerThread::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {

    switch(method) {
        
        case JSONMethod::MessageFromPlayer: 
        {
            RPZMessage message(data.toHash());
            message.setOwnership(*this->_getUser(target)); //force corresponding user to it then store it
            this->_interpretMessage(target, message);
        }
        break;

        case JSONMethod::AudioStreamUrlChanged: {
            this->_sendToAllButSelf(target, JSONMethod::AudioStreamUrlChanged, data);
        }
        break;

        case JSONMethod::AudioStreamPlayingStateChanged: {
            this->_sendToAllButSelf(target, JSONMethod::AudioStreamPlayingStateChanged, data);
        }
        break;

        case JSONMethod::AudioStreamPositionChanged: {
            this->_sendToAllButSelf(target, JSONMethod::AudioStreamPositionChanged, data);
        }
        break;

        case JSONMethod::AskForAssets: {   
            auto list = data.toList();
            QList<RPZAssetHash> requestedIds;
            for(auto &var : list) requestedIds.append(var.toString());

            for(auto &id : requestedIds) {
                auto package = AssetsDatabase::get()->prepareAssetPackage(id);
                target->sendJSON(JSONMethod::RequestedAsset, package);
            }

        }
        break;

        case JSONMethod::MapChanged:{
            auto hash = data.toHash();
            this->_broadcastMapChanges(hash, target);
        }
        break;

        case JSONMethod::Handshake: {   
            
            //prepare
            auto targetUser = this->_getUser(target);
            auto handshakePkg = RPZHandshake(data.toHash());
            
            //check versions with server, if different, reject
            auto serverVersion = QString(APP_CURRENT_VERSION);
            auto clientVersion = handshakePkg.clientVersion();
            if(clientVersion != serverVersion) {
                target->sendJSON(JSONMethod::ServerStatus, 
                    QString("Incompatibilité de version entre client et serveur : v%1 (client) / v%2 (serveur) !")
                    .arg(clientVersion)
                    .arg(serverVersion)
                );
                return;
            }

            //change the requested username if already exists
            auto chosenName = handshakePkg.requestedUsername();
            auto adapted = MessageInterpreter::usernameToCommandCompatible(chosenName);
            if(this->_formatedUsernamesByUser.contains(adapted)) {
                chosenName = chosenName + targetUser->color().name();
            }
            targetUser->setName(chosenName);

            //tell other users this one exists
            this->_broadcastUsers();

            //send user object to socket
            this->_tellUserHisIdentity(target);

            //send history to the client
            this->_sendStoredMessages(target);

            //ask for host map history
            if(target == this->_hostSocket)  {
                this->_askHostForMapHistory();
            } else {
                this->_sendMapHistory(target);
            }
        }
        break;

        default:
            break;
        
    }

}

//
// HIGH Helpers
//

void RPZServerThread::_tellUserHisIdentity(JSONSocket* socket) {
    auto serialized = this->_getUser(socket);
    socket->sendJSON(JSONMethod::AckIdentity, *serialized);
}

void RPZServerThread::_sendStoredMessages(JSONSocket * clientSocket) {
    
    //send messages...
    auto messagesToSend = this->_messages.toVList();
    auto method = JSONMethod::ChatLogHistory;
    clientSocket->sendJSON(method, messagesToSend);

    //log
    auto countMsgs = this->_messages.count();
    auto logMsg = QString("%1 stored messages sent to \"%2\"")
                        .arg(countMsgs)
                        .arg(clientSocket->socket()->peerAddress().toString());
    JSONSocket::_debugLog("RPZServerThread", method, logMsg);
}


void RPZServerThread::_broadcastUsers() {
    
    //reset registered username list
    this->_formatedUsernamesByUser.clear();

    //refill
    for(auto &user : this->_usersById) {
        auto formated = MessageInterpreter::usernameToCommandCompatible(user.name());
        this->_formatedUsernamesByUser.insert(formated, &user);
    }

    //send data
    auto method = JSONMethod::LoggedPlayersChanged;
    auto toSend =  this->_usersById.toVList();
    this->_sendToAll(method, toSend);

    //log
    auto userCount = this->_usersById.size();
    auto msgLog = QString("Now %1 clients logged").arg(userCount);
    JSONSocket::_debugLog("RPZServerThread", method, msgLog);
}

void RPZServerThread::_askHostForMapHistory() {
    this->_hostSocket->sendJSON(JSONMethod::AskForHostMapHistory, QStringList());
}

void RPZServerThread::_alterIncomingPayloadWithUpdatedOwners(AtomsWielderPayload &wPayload, JSONSocket * senderSocket) {
    auto defaultOwner = this->_getUser(senderSocket); 
    wPayload.updateEmptyUser(*defaultOwner);
}

void RPZServerThread::_broadcastMapChanges(QVariantHash &payload, JSONSocket * senderSocket) {

    auto aPayload = Payloads::autoCast(payload);

    if(auto wPayload = aPayload.dynamicCast<AtomsWielderPayload>()) {
        this->_alterIncomingPayloadWithUpdatedOwners(*wPayload, senderSocket);
    }

    //save for history
    this->_hints->queueAlteration(*aPayload, false); //prevent sending alterations to UI (different threads !)

    //add source for outer calls
    auto source = this->_hints->source();
    aPayload->changeSource(source);

    //send to registered users...
    this->_sendToAllButSelf(senderSocket, JSONMethod::MapChanged, *aPayload);
}

void RPZServerThread::_sendMapHistory(JSONSocket * clientSocket) {
    auto allAtoms = this->_hints->atoms();
    ResetPayload payload(allAtoms);
    clientSocket->sendJSON(JSONMethod::MapChanged, payload);

}

void RPZServerThread::_interpretMessage(JSONSocket* sender, RPZMessage &msg){
    
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
            auto recipients = MessageInterpreter::findRecipentsFromText(textCommand);

            //iterate
            QList<QString> notFound;
            for(auto &recipient : recipients) {
                
                //find user from recipident
                auto user = this->_formatedUsernamesByUser[recipient];
                if(!user) {
                    notFound.append(recipient);
                    continue;
                }

                //send to recipient user
                auto textOnly = MessageInterpreter::sanitizeText(textCommand);
                
                //create a new message 
                auto newMessage = RPZMessage(textOnly, MessageInterpreter::Whisper);
                newMessage.setOwnership(msg.owner());

                //send new message
                user->networkSocket()->sendJSON(JSONMethod::MessageFromPlayer, newMessage);
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
            this->_sendToAllButSelf(sender, JSONMethod::MessageFromPlayer, msg);

        }
        break;
    }

    //set ack if no specific reponse set
    if(!response.answerer()) response = RPZResponse(msgId);

    //send response
    sender->sendJSON(JSONMethod::ServerResponse, response);
}

//
//
//

//
// LOW Helpers
//

void RPZServerThread::_sendToAllButSelf(JSONSocket * senderSocket, const JSONMethod &method, const QVariant &data) {
    for(auto &user : this->_usersById) {
        
        //prevent self send
        if(user.networkSocket() == senderSocket) {
            continue;
        } 

        //send to others
        user.networkSocket()->sendJSON(method, data);
    }
}

void RPZServerThread::_sendToAll(const JSONMethod &method, const QVariant &data) {
    for(auto &user : this->_usersById) {
        user.networkSocket()->sendJSON(method, data);
    }
}


RPZUser* RPZServerThread::_getUser(JSONSocket* socket) {
    const auto id = this->_idsByClientSocket[socket];
    return &this->_usersById[id];
}