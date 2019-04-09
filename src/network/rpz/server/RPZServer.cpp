#include "RPZServer.h" 

RPZServer::RPZServer(QObject* parent) : QTcpServer(parent), _hints(new MapHint) {  };

RPZServer::~RPZServer() {
    //ended server
    qDebug() << "RPZServer : Server ending !";
    this->close();
}

void RPZServer::run() { 

    qDebug() << "RPZServer : Starting server...";

    auto result = this->listen(QHostAddress::Any, std::stoi(UPNP_DEFAULT_TARGET_PORT));

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
                auto message = RPZMessage::fromVariantHash(data.toHash());
                message.setUser(this->_getUser(target));
                this->_messages.insert(message.id(), message);

                //push to all sockets
                this->_broadcastMessage(message);
            }
            break;
        case JSONMethod::MapChanged:
        case JSONMethod::HostMapHistory:
            {
                const auto history = data.toList();
                this->_broadcastMapChanges(history, target);
            }
            break;
        case JSONMethod::PlayerHasUsername:
            {   
                //bind username to socket
                const auto dn = data.toList()[0].toString();
                this->_getUser(target)->setName(dn);

                //tell other users this one exists
                this->_broadcastUsers();

                //send history to the client
                this->_sendStoredMessages(target);

                //ask for host map history
                if(target == this->_hostSocket)  {
                    this->_askHostForMapHistory();
                }
            }
            break;
        default:
            qWarning() << "RPZServer : unknown method from JSON !";
    }

}

void RPZServer::_sendStoredMessages(JSONSocket * clientSocket) {

    //message...
    auto countMsgs = this->_messages.size();
    auto serialized = this->_serializeMessages();
    clientSocket->sendJSON(JSONMethod::ChatLogHistory, serialized);
    
    qDebug() << "RPZServer :" << countMsgs << " stored messages sent to " << clientSocket->socket()->peerAddress().toString();
}

void RPZServer::_broadcastMessage(RPZMessage &messageToBroadcast) {

    for(auto &user : this->_usersById) {
        user.jsonHelper()->sendJSON(JSONMethod::MessageFromPlayer, messageToBroadcast.toVariantHash());
    }

    qDebug() << "RPZServer : Broadcasted message to " << this->_usersById.size() << " clients";
}

void RPZServer::_broadcastUsers() {

    auto serialized = this->_serializeUsers();

    for(auto &user : this->_usersById) {
        user.jsonHelper()->sendJSON(JSONMethod::LoggedPlayersChanged, serialized);
    }

    qDebug() << "RPZServer : Now " << this->_usersById.size() << " clients logged";
}

void RPZServer::_askHostForMapHistory() {
    this->_hostSocket->sendJSON(JSONMethod::AskForHostMapHistory, QStringList());
}

void RPZServer::_broadcastMapChanges(const QVariantList &changes, JSONSocket * senderSocket) {

    //send...
    for(auto &user : this->_usersById) {
        if(user.jsonHelper() == senderSocket) continue; //prevent self send
        user.jsonHelper()->sendJSON(JSONMethod::MapChanged, changes);
    }

}

QVariantHash RPZServer::_serializeUsers() {
    QVariantHash base;

    for (auto &user : this->_usersById) {
        base.insert(user.id().toString(), user.toVariantHash());
    }

    return base;
}

QVariantList RPZServer::_serializeMessages() {
    QVariantList base;

    for (auto &msg : this->_messages) {
        base.append(msg.toVariantHash());
    }

    return base;
}


RPZUser* RPZServer::_getUser(JSONSocket* socket) {
    const auto id = this->_idsByClientSocket[socket];
    return &this->_usersById[id];
}
