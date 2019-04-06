#include "RPZServer.h" 

RPZServer::RPZServer() { };

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

void RPZServer::_broadcastMessage(QString messageToBroadcast) {

    for(auto socket : this->_clientSockets) {
        socket->sendJSON(JSONMethod::MessageFromPlayer, QStringList(messageToBroadcast));
    }

    qDebug() << "Chat Server : Broadcasted message to " << this->_clientSockets.size() << " clients";
}

void RPZServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = new JSONSocket("Chat Server", this->_server->nextPendingConnection());
        this->_clientSockets.insert(clientSocket, clientSocket);

        //check if host
        if(clientSocket->socket()->localAddress() == QHostAddress::LocalHost) {
            this->_hostSocket = clientSocket;
        }
        
        //clear !
        QObject::connect(
            clientSocket->socket(), &QAbstractSocket::disconnected,
            [&, clientSocket]() {
                
                //remove socket
                this->_clientSockets.remove(clientSocket);
                this->_clientDisplayNames.remove(clientSocket);
                clientSocket->deleteLater();

                //desalocate host
                if(this->_hostSocket == clientSocket) {
                    this->_hostSocket = nullptr;
                }

                //tell other clients that the user is gone
                this->_broadcastUsers();

            }
        );

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
                this->_clientDisplayNames[target] = dn;

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
    for(auto socket : this->_clientSockets) {
        socket->sendJSON(JSONMethod::LoggedPlayersChanged, QStringList(this->_clientDisplayNames.values()));
    }
}

QString RPZServer::_getSocketDisplayName(JSONSocket * clientSocket) {
    return this->_clientDisplayNames.contains(clientSocket) ? 
            this->_clientDisplayNames[clientSocket] : 
            clientSocket->socket()->peerAddress().toString();
}