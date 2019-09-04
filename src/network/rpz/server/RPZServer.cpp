#include "RPZServer.h" 

RPZServer::RPZServer() {

    qRegisterMetaType<JSONMethod>("JSONMethod");

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

void RPZServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = new JSONSocket("RPZServer", this->nextPendingConnection());
        
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

    //remove socket
    const auto idToRemove = this->_idsByClientSocket.take(disconnectedSocket);
    this->_usersById.remove(idToRemove);

    //desalocate host
    if(this->_hostSocket == disconnectedSocket) {
        this->_hostSocket = nullptr;
    }

    auto disconnectingAddress = disconnectedSocket->socket()->peerAddress().toString();
    qDebug() << "RPZServer : " << disconnectingAddress << " disconnected !";

    delete disconnectedSocket;

    //tell other clients that the user is gone
    this->_broadcastUsers();

}

void RPZServer::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {

    switch(method) {
        
        case JSONMethod::MessageFromPlayer: 
        {
            RPZMessage message(data.toHash());
            message.setOwnership(this->_getUser(target)); //force corresponding user to it then store it
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
            auto aPayload = Payloads::autoCast(data.toHash());
            this->_broadcastMapChanges(*aPayload, target);
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
                chosenName = chosenName + targetUser.color().name();
            }
            targetUser.setName(chosenName);

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

void RPZServer::_tellUserHisIdentity(JSONSocket* socket) {
    auto serialized = this->_getUser(socket);
    socket->sendJSON(JSONMethod::AckIdentity, serialized);
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


void RPZServer::_broadcastUsers() {
    
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
    JSONSocket::_debugLog("RPZServer", method, msgLog);
}

void RPZServer::_askHostForMapHistory() {
    this->_hostSocket->sendJSON(JSONMethod::AskForHostMapHistory, QStringList());
}

void RPZServer::_alterIncomingPayloadWithUpdatedOwners(AtomsWielderPayload &wPayload, JSONSocket * senderSocket) {
    
    auto defaultOwner = this->_getUser(senderSocket); 
    auto updated = wPayload.updateEmptyUser(defaultOwner);
    
    //if sender sent no user atoms
    if(updated.count()) {

        //tell him that he owns them now
        OwnerChangedPayload changedOwner(updated, defaultOwner);
        auto source = this->_hints->source();
        changedOwner.changeSource(source);

        //send...
        this->_sendToAllButSelf(senderSocket, JSONMethod::MapChanged, changedOwner);

    }
}

void RPZServer::_broadcastMapChanges(AlterationPayload &payload, JSONSocket * senderSocket) {

    if(auto wPayload = dynamic_cast<AtomsWielderPayload*>(&payload)) {
        this->_alterIncomingPayloadWithUpdatedOwners(*wPayload, senderSocket);
    }

    //save for history
    this->_hints->handleAlterationRequest(payload);

    //add source for outer calls
    auto source = this->_hints->source();
    payload.changeSource(source);

    //send to registered users but sender...
    this->_sendToAllButSelf(senderSocket, JSONMethod::MapChanged, payload);
}

void RPZServer::_sendMapHistory(JSONSocket * clientSocket) {
    auto allAtoms = this->_hints->atoms();
    ResetPayload payload(allAtoms);
    clientSocket->sendJSON(JSONMethod::MapChanged, payload);

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

void RPZServer::_sendToAllButSelf(JSONSocket * senderSocket, const JSONMethod &method, const QVariant &data) {
    for(auto &user : this->_usersById) {
        
        //prevent self send
        if(user.networkSocket() == senderSocket) {
            continue;
        } 

        //send to others
        user.networkSocket()->sendJSON(method, data);
    }
}

void RPZServer::_sendToAll(const JSONMethod &method, const QVariant &data) {
    for(auto &user : this->_usersById) {
        user.networkSocket()->sendJSON(method, data);
    }
}


RPZUser& RPZServer::_getUser(JSONSocket* socket) {
    const auto id = this->_idsByClientSocket[socket];
    return this->_usersById[id];
}