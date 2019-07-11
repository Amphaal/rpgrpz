#include "RPZServer.h" 

RPZServer::RPZServer(QObject* parent) : QTcpServer(parent),  _hints(new AtomsStorage(AlterationPayload::Source::Network)) { };

RPZServer::~RPZServer() {
    //ended server
    qDebug() << "RPZServer : Server ending !";
    this->close();
}

void RPZServer::run() { 

    qDebug() << "RPZServer : Starting server...";

    auto result = this->listen(QHostAddress::Any, AppContext::UPNP_DEFAULT_TARGET_PORT.toInt());

    if(!result) {
        qWarning() << "RPZServer : Error while starting to listen >> " + this->errorString();
        emit error();
        return;
    } else {
        qDebug() << "RPZServer : Succesfully listening !";
        emit listening();
    }

    //connect to new connections
    QObject::connect(
        this, &QTcpServer::newConnection, 
        this, &RPZServer::_onNewConnection
    );

};

void RPZServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = new JSONSocket(this, "RPZServer", this->nextPendingConnection());
        
        //create new user
        auto user = RPZUser(clientSocket);

        //check if host
        if(clientSocket->socket()->localAddress().isLoopback()) {
            this->_hostSocket = clientSocket;
            user.setRole(RPZUser::Role::Host);
        }

        //add to internal lists
        this->_usersById.insert(user.id(), user);
        this->_idsByClientSocket.insert(clientSocket, user.id());
        
        //clear on client disconnect
        QObject::connect(
            clientSocket, &JSONSocket::disconnected,
            this, &RPZServer::_onDisconnect
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

void RPZServer::_onDisconnect() {

    auto clientSocket = (JSONSocket*)this->sender();

    //remove socket
    const auto idToRemove = this->_idsByClientSocket.take(clientSocket);
    this->_usersById.remove(idToRemove);

    //desalocate host
    if(this->_hostSocket == clientSocket) {
        this->_hostSocket = nullptr;
    }

    qDebug() << "RPZServer : " << clientSocket->socket()->peerAddress().toString() << " disconnected !";

    clientSocket->deleteLater();

    //tell other clients that the user is gone
    this->_broadcastUsers();

}

void RPZServer::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {

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
            QList<RPZAssetId> requestedIds;
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

void RPZServer::_tellUserHisIdentity(JSONSocket* socket) {
    auto serialized = this->_getUser(socket);
    socket->sendJSON(JSONMethod::AckIdentity, *serialized);
}

void RPZServer::_sendStoredMessages(JSONSocket * clientSocket) {
    //message...
    auto countMsgs = this->_messages.count();
    clientSocket->sendJSON(JSONMethod::ChatLogHistory, this->_messages.toVList());
    qDebug() << "RPZServer :" << countMsgs << "stored messages sent to" << clientSocket->socket()->peerAddress().toString();
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
    this->_sendToAll(JSONMethod::LoggedPlayersChanged, this->_usersById.toVList());
    qDebug() << "RPZServer : Now" << this->_usersById.size() << "clients logged";
}

void RPZServer::_askHostForMapHistory() {
    this->_hostSocket->sendJSON(JSONMethod::AskForHostMapHistory, QStringList());
}

void RPZServer::_alterIncomingPayloadWithUpdatedOwners(AtomsWielderPayload &wPayload, JSONSocket * senderSocket) {
    auto defaultOwner = this->_getUser(senderSocket); 
    wPayload.updateEmptyUser(*defaultOwner);
}

void RPZServer::_broadcastMapChanges(QVariantHash &payload, JSONSocket * senderSocket) {

    auto aPayload = Payloads::autoCast(payload);

    if(auto wPayload = aPayload.dynamicCast<AtomsWielderPayload>()) {
        this->_alterIncomingPayloadWithUpdatedOwners(*wPayload, senderSocket);
    }

    //save for history
    this->_hints->handleAlterationRequest(payload);

    //add source for outer calls
    aPayload->changeSource(this->_hints->source());

    //send to registered users...
    this->_sendToAllButSelf(senderSocket, JSONMethod::MapChanged, payload);
}

void RPZServer::_sendMapHistory(JSONSocket * clientSocket) {
    auto allAtoms = this->_hints->atoms();
    auto payload = ResetPayload(allAtoms);
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
                user->jsonHelper()->sendJSON(JSONMethod::MessageFromPlayer, newMessage);
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
        if(user.jsonHelper() == senderSocket) {
            continue;
        } 

        //send to others
        user.jsonHelper()->sendJSON(method, data);
    }
}

void RPZServer::_sendToAll(const JSONMethod &method, const QVariant &data) {
    for(auto &user : this->_usersById) {
        user.jsonHelper()->sendJSON(method, data);
    }
}


RPZUser* RPZServer::_getUser(JSONSocket* socket) {
    const auto id = this->_idsByClientSocket[socket];
    return &this->_usersById[id];
}