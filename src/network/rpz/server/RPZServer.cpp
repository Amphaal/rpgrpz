#include "RPZServer.h" 

RPZServer::RPZServer(QObject* parent) : QTcpServer(parent), 
                                        _hints(new MapHint(AlterationPayload::Source::Network)) { };


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
        qDebug() << "RPZServer : New connection from " << newIp;

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

    clientSocket->deleteLater();

    //tell other clients that the user is gone
    this->_broadcastUsers();

}

void RPZServer::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {

    switch(method) {
        case JSONMethod::MessageFromPlayer: 
        {
                //get message, add corresponding user to it then store it
                RPZMessage message(data.toHash());
                message.setOwnership(*this->_getUser(target));

                this->_interpretMessage(target, message);
            }
            break;
        case JSONMethod::AskForAsset: {   

                auto assetId = data.toString();
                auto package = AssetsDatabase::get()->prepareAssetPackage(assetId);
                target->sendJSON(JSONMethod::RequestedAsset, package);
                
            }
            break;
        case JSONMethod::MapChanged:{
                this->_broadcastMapChanges(data.toHash(), target);
            }
            break;
        case JSONMethod::PlayerHasUsername: {   

                //bind username to socket
                const auto dn = data.toList()[0].toString();
                this->_getUser(target)->setName(dn);

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
            qWarning() << "RPZServer : unknown method from JSON !";
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
    qDebug() << "RPZServer :" << countMsgs << " stored messages sent to " << clientSocket->socket()->peerAddress().toString();
}


void RPZServer::_broadcastUsers() {
    this->_sendToAll(JSONMethod::LoggedPlayersChanged, this->_usersById.toVList());
    qDebug() << "RPZServer : Now " << this->_usersById.size() << " clients logged";
}

void RPZServer::_askHostForMapHistory() {
    this->_hostSocket->sendJSON(JSONMethod::AskForHostMapHistory, QStringList());
}

AlterationPayload RPZServer::_alterIncomingPayloadWithUpdatedOwners(QVariantHash &payload, JSONSocket * senderSocket) {

    auto aPayload = Payload::autoCast(payload);
    auto type = aPayload->type();

    //no need to modify anything if not a build operation
    if(!aPayload->requiresGraphicsItemBuild()) {
        delete aPayload;
        return payload;
    }

    //get sender identity
    auto defaultOwner = this->_getUser(senderSocket); 

    //override ownership on absent owner data
    QVector<RPZAtom> updatedAtoms;
    for(auto &atomRaw : aPayload->alterationByAtomId()) {
        
        RPZAtom atom(atomRaw.toHash());
        auto owner = atom.owner();

        if(owner.isEmpty()) atom.setOwnership(*defaultOwner);
        
        updatedAtoms.append(atom);
    }
    delete aPayload;

    //return altered
    switch(type) {
        case AlterationPayload::Alteration::Added:
            return AddedPayload(updatedAtoms.first());
        case AlterationPayload::Alteration::Reset:
            return ResetPayload(updatedAtoms);
    }

    //default return
    return payload;
}

void RPZServer::_broadcastMapChanges(QVariantHash &payload, JSONSocket * senderSocket) {

    //cast
    auto aPayload = this->_alterIncomingPayloadWithUpdatedOwners(payload, senderSocket);

    //save for history
    this->_hints->alterScene(aPayload);

    //add source for outer calls
    aPayload.changeSource(this->_hints->source());

    //send to registered users...
    this->_sendToAllButSelf(senderSocket, JSONMethod::MapChanged, aPayload);

}

void RPZServer::_sendMapHistory(JSONSocket * clientSocket) {
    auto payload = ResetPayload(this->_hints->atoms());
    clientSocket->sendJSON(JSONMethod::MapChanged, payload);

}

void RPZServer::_interpretMessage(JSONSocket* sender, RPZMessage &msg){
    
    auto msgId = msg.id();
    RPZResponse response;

    switch(msg.commandType()) {
        
        //on unknown command
        case MessageInterpreter::Unknown: {
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
            //TODO
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
    if(response.answerer().isNull()) response = RPZResponse(msgId);

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