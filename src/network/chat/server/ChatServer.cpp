#include "ChatServer.h" 

ChatServer::ChatServer() { };

void ChatServer::run() { 

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


void ChatServer::_sendStoredMessages(JSONSocket * clientSocket) {

    //message...
    auto countMsgs = this->_messages.size();
    clientSocket->sendJSON("messages_history", this->_messages);
    
    qDebug() << "Chat Server :" << countMsgs << " stored messages sent to " << clientSocket->socket()->peerAddress().toString();
}

void ChatServer::_broadcastMessage(QString messageToBroadcast) {

    for(auto socket : this->_clientSockets) {
        socket->sendJSON("new_message", QStringList(messageToBroadcast));
    }

    qDebug() << "Chat Server : Broadcasted message to " << this->_clientSockets.size() << " clients";
}

void ChatServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = new JSONSocket("Chat Server", this->_server->nextPendingConnection());
        this->_clientSockets.insert(clientSocket, clientSocket);
        
        //clear !
        QObject::connect(
            clientSocket->socket(), &QAbstractSocket::disconnected,
            [&, clientSocket]() {
                this->_clientSockets.remove(clientSocket);
                this->_clientDisplayNames.remove(clientSocket);
                clientSocket->deleteLater();
            }
        );

        QObject::connect(
            clientSocket, &JSONSocket::JSONReceived,
            [&](JSONSocket * wrapper, QString method, QVariant data) {
                this->_routeIncomingJSON(wrapper, method, data);
            } 
        );

        //signals new connection
        auto newIp = clientSocket->socket()->peerAddress().toString();
        emit newConnectionReceived(newIp.toStdString());
        qDebug() << "Chat Server : New connection from " << newIp;

        this->_sendStoredMessages(clientSocket);

}

void ChatServer::_routeIncomingJSON(JSONSocket * wrapper, QString method, QVariant data) {

    if (method == "new_message") {
        auto rawMsg = data.toList()[0].toString();
        auto sockUN = this->_getSocketDisplayName(wrapper);
        auto message = formatChatMessage(sockUN, rawMsg);

        this->_messages << message;

        //push to all sockets
        this->_broadcastMessage(message);
    
    } else if (method == "display_name") {
        
        //bind username to socket
        auto dn = data.toList()[0].toString();
        this->_clientDisplayNames[wrapper] = dn;

         this->_broadcastUsers();
    
    } else {
        qWarning() << "Chat Server : unknown method from JSON !";
    }

}

void ChatServer::_broadcastUsers() {
    for(auto socket : this->_clientSockets) {
        socket->sendJSON("logged_users", QStringList(this->_clientDisplayNames.values()));
    }
}

QString ChatServer::_getSocketDisplayName(JSONSocket * clientSocket) {
    return this->_clientDisplayNames.contains(clientSocket) ? 
            this->_clientDisplayNames[clientSocket] : 
            clientSocket->socket()->peerAddress().toString();
}