#include "RPZServer.h" 

RPZServer::RPZServer() { };

void RPZServer::run() { 

    this->_server = new QTcpServer;
    this->_hints = new MapHint;

    qDebug() << "Chat Server : Starting server...";

    auto result = this->_server->listen(QHostAddress::Any, std::stoi(UPNP_DEFAULT_TARGET_PORT));

    if(!result) {
        qWarning() << "Chat Server : Error while starting to listen >> " + this->_server->errorString();
        return;
    } else {
        qDebug() << "Chat Server : Succesfully listening !";
    }

    //connect to new connections
    QObject::connect(this->_server, &QTcpServer::newConnection, [&](){
        this->_onNewConnection();
    });

    this->exec();

    //ended server
    qDebug() << "Chat Server : Server ending !";
    this->_server->close();
};

void RPZServer::_sendStoredMessages(JSONSocket * clientSocket) {

    //message...
    auto countMsgs = this->_messages.size();
    clientSocket->sendJSON(JSONMethod::ChatLogHistory, this->_messages);
    
    qDebug() << "Chat Server :" << countMsgs << " stored messages sent to " << clientSocket->socket()->peerAddress().toString();
}

void RPZServer::_broadcastMessage(const QString &messageToBroadcast) {

    for(auto &socket : this->_clientSocketsById) {
        socket->sendJSON(JSONMethod::MessageFromPlayer, QStringList(messageToBroadcast));
    }

    qDebug() << "Chat Server : Broadcasted message to " << this->_clientSocketsById.size() << " clients";
}

void RPZServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = new JSONSocket("Chat Server", this->_server->nextPendingConnection());
        
        //store it
        const auto newId = QUuid::createUuid();
        this->_clientSocketsById.insert(newId, clientSocket);
        this->_idsByClientSocket.insert(clientSocket, newId);

        //check if host
        const auto la = clientSocket->socket()->localAddress();
        if(la == QHostAddress::LocalHost || la == QHostAddress::LocalHostIPv6) {
            this->_hostSocket = clientSocket;

            this->_askHostForMapHistory();
        }
        
        //clear on client disconnect
        QObject::connect(
            clientSocket->socket(), &QAbstractSocket::disconnected,
            [&, clientSocket]() {
                
                //remove socket
                const auto idToRemove = this->_idsByClientSocket.take(clientSocket);
                this->_clientSocketsById.remove(idToRemove);
                this->_clientDisplayNames.remove(idToRemove);

                //desalocate host
                if(this->_hostSocket == clientSocket) {
                    this->_hostSocket = nullptr;
                }

                clientSocket->deleteLater();

                //tell other clients that the user is gone
                this->_broadcastUsers();

            }
        );

        //on data reception
        QObject::connect(
            clientSocket, &JSONSocket::JSONReceived,
            [&](JSONSocket* target, const JSONMethod &method, const QVariant &data) {
                this->_routeIncomingJSON(target, method, data);
            } 
        );

        //signals new connection
        auto newIp = clientSocket->socket()->peerAddress().toString();
        emit newConnectionReceived(newIp.toStdString());
        qDebug() << "Chat Server : New connection from " << newIp;

}

void RPZServer::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {

    switch(method) {
        case JSONMethod::MessageFromPlayer:
            {
                const auto rawMsg = data.toList()[0].toString();
                const auto sockUN = this->_getSocketDisplayName(target);
                const auto message = formatChatMessage(sockUN, rawMsg);

                this->_messages << message;

                //push to all sockets
                this->_broadcastMessage(message);
            }
            break;
        case JSONMethod::HostMapHistory:
            {
                const auto history = data.toList();
                this->_broadcastMapChanges(history);
            }
            break;
        case JSONMethod::PlayerHasUsername:
            {   
                //bind username to socket
                const auto dn = data.toList()[0].toString();
                this->_clientDisplayNames[this->_idsByClientSocket[target]] = dn;

                //tell other users this one exists
                this->_broadcastUsers();

                //send history to the client
                this->_sendStoredMessages(target);
            }
            break;
        default:
            qWarning() << "Chat Server : unknown method from JSON !";
    }

}

void RPZServer::_broadcastUsers() {
    for(auto &socket : this->_clientSocketsById) {
        socket->sendJSON(JSONMethod::LoggedPlayersChanged, QStringList(this->_clientDisplayNames.values()));
    }
}

void RPZServer::_askHostForMapHistory() {
    this->_hostSocket->sendJSON(JSONMethod::AskForHostMapHistory, QStringList());
}

QString RPZServer::_getSocketDisplayName(JSONSocket * clientSocket) {
    return this->_idsByClientSocket.contains(clientSocket) ? 
            this->_clientDisplayNames[this->_idsByClientSocket[clientSocket]] : 
            clientSocket->socket()->peerAddress().toString();
}


void RPZServer::_broadcastMapChanges(const QVariantList &changes) {

    //send...
    for(auto &socket : this->_clientSocketsById) {
        if(socket == this->_hostSocket) continue;
        socket->sendJSON(JSONMethod::HostMapChanged, changes);
    }

}