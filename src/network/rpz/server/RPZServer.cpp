#include "RPZServer.h" 

RPZServer::RPZServer(MapView* mv) : _mv(mv) { };

void RPZServer::run() { 

    this->_server = new QTcpServer;

    qDebug() << "Chat Server : Starting server...";

    auto result = this->_server->listen(QHostAddress::Any, std::stoi(UPNP_DEFAULT_TARGET_PORT));

    if(!result) {
        qWarning() << "Chat Server : Error while starting to listen >> " + this->_server->errorString();
        return;
    } else {
        qDebug() << "Chat Server : Succesfully listening !";
    }

    //connect to map changes
    QObject::connect(
        this->_mv, &MapView::mapElementsAltered,
        [&](QList<Asset> elements, MapView::MapElementEvtState state) {
            this->_onMapChanged(elements, state);
        }
    );

    //connect to new connections
    QObject::connect(this->_server, &QTcpServer::newConnection, [&](){
        this->_onNewConnection();
    });

    this->exec();

    //ended server
    qDebug() << "Chat Server : Server ending !";
    this->_server->close();
};

void RPZServer::_onMapChanged(QList<Asset> elements, MapView::MapElementEvtState state) {

    if(!this->_clientSocketsById.size()) return;

    auto toSend = this->_mv->packageForNetworkSend(elements, state);

    //send...
    for(auto socket : this->_clientSocketsById) {
        //if(socket == this->_hostSocket) continue; //TODO reactivate
        socket->sendJSON(JSONMethod::HostMapChanged, toSend);
    }

}

void RPZServer::_sendMapHistory() {
    if(!this->_clientSocketsById.size()) return;

    //TODO
}

void RPZServer::_sendStoredMessages(JSONSocket * clientSocket) {

    //message...
    auto countMsgs = this->_messages.size();
    clientSocket->sendJSON(JSONMethod::ChatLogHistory, this->_messages);
    
    qDebug() << "Chat Server :" << countMsgs << " stored messages sent to " << clientSocket->socket()->peerAddress().toString();
}

void RPZServer::_broadcastMessage(QString messageToBroadcast) {

    for(auto socket : this->_clientSocketsById) {
        socket->sendJSON(JSONMethod::MessageFromPlayer, QStringList(messageToBroadcast));
    }

    qDebug() << "Chat Server : Broadcasted message to " << this->_clientSocketsById.size() << " clients";
}

void RPZServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = new JSONSocket("Chat Server", this->_server->nextPendingConnection());
        
        //store it
        auto newId = QUuid::createUuid();
        this->_clientSocketsById.insert(newId, clientSocket);
        this->_idsByClientSocket.insert(clientSocket, newId);

        //check if host
        auto la = clientSocket->socket()->localAddress();
        if(la == QHostAddress::LocalHost || la == QHostAddress::LocalHostIPv6) {
            this->_hostSocket = clientSocket;
        }
        
        //clear on client disconnect
        QObject::connect(
            clientSocket->socket(), &QAbstractSocket::disconnected,
            [&, clientSocket]() {
                
                //remove socket
                auto idToRemove = this->_idsByClientSocket.take(clientSocket);
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
            [&](JSONSocket* target, JSONMethod method, QVariant data) {
                this->_routeIncomingJSON(target, method, data);
            } 
        );

        //signals new connection
        auto newIp = clientSocket->socket()->peerAddress().toString();
        emit newConnectionReceived(newIp.toStdString());
        qDebug() << "Chat Server : New connection from " << newIp;

}

void RPZServer::_routeIncomingJSON(JSONSocket* target, JSONMethod method, QVariant data) {

    switch(method) {
        case JSONMethod::MessageFromPlayer:
            {
                auto rawMsg = data.toList()[0].toString();
                auto sockUN = this->_getSocketDisplayName(target);
                auto message = formatChatMessage(sockUN, rawMsg);

                this->_messages << message;

                //push to all sockets
                this->_broadcastMessage(message);
            }
            break;
        case JSONMethod::PlayerHasUsername:
            {   
                //bind username to socket
                auto dn = data.toList()[0].toString();
                this->_clientDisplayNames[this->_idsByClientSocket[target]] = dn;

                //tell other users this one exists
                this->_broadcastUsers();

                //send history to the client
                this->_sendStoredMessages(target);

                this->_sendMapHistory();
            }
            break;
        default:
            qWarning() << "Chat Server : unknown method from JSON !";
    }

}

void RPZServer::_broadcastUsers() {
    for(auto socket : this->_clientSocketsById) {
        socket->sendJSON(JSONMethod::LoggedPlayersChanged, QStringList(this->_clientDisplayNames.values()));
    }
}

QString RPZServer::_getSocketDisplayName(JSONSocket * clientSocket) {
    return this->_idsByClientSocket.contains(clientSocket) ? 
            this->_clientDisplayNames[this->_idsByClientSocket[clientSocket]] : 
            clientSocket->socket()->peerAddress().toString();
}