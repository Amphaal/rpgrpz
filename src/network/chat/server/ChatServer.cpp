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

void ChatServer::_sendJSONtoSocket(QTcpSocket * clientSocket, QJsonDocument doc) {

    //send welcome message
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);

    //send...
    out << doc.toBinaryData();
    auto written = clientSocket->write(block);

    //wait end send
    clientSocket->waitForBytesWritten();
}

void ChatServer::_sendStoredMessages(QTcpSocket * clientSocket) {

    //message...
    auto countMsgs = this->_messages.size();
    auto json_msgs = QJsonDocument::fromVariant(this->_messages);
    auto msgsAsString = QString(json_msgs.toJson(QJsonDocument::Compact));
    auto peeet = QString("{ \"messages_history\":" + msgsAsString + "}");
    auto json_payload = QJsonDocument::fromJson(peeet.toUtf8());

    this->_sendJSONtoSocket(clientSocket, json_payload);
    
    qDebug() << "Chat Server :" << countMsgs << " stored messages sent to " << clientSocket->peerAddress().toString();
}

void ChatServer::_broadcastMessage(QString messageToBroadcast) {

    auto peeet = QString("{ \"new_message\": \"" + messageToBroadcast + "\"}");
    auto json_payload = QJsonDocument::fromJson(peeet.toUtf8());

    for(auto socket : this->_clientSockets) {
        this->_sendJSONtoSocket(socket, json_payload);
    }

    qDebug() << "Chat Server : Broadcasted message to " << QString(this->_clientSockets.size()) << " clients";
}

void ChatServer::_handleIncomingMessages(QTcpSocket * clientSocket) {

    QDataStream in;
    in.setVersion(QDataStream::Qt_5_12);
    in.setDevice(clientSocket);

    in.startTransaction();

    QString receivedMessage; 
    in >> receivedMessage;
    this->_messages << receivedMessage;

    if (!in.commitTransaction()) {
        qWarning() << "Chat Server : issue while reading incoming message";  
        return;
    }

    //push to all sockets
    this->_broadcastMessage(receivedMessage);

    qDebug() << "Chat Server : message received from " << clientSocket->peerAddress().toString() << " >> " << receivedMessage;
}

void ChatServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = this->_server->nextPendingConnection();
        this->_clientSockets.insert(clientSocket, clientSocket);

        QObject::connect(
            clientSocket, &QAbstractSocket::disconnected,
            [&, clientSocket]() {
                this->_clientSockets.remove(clientSocket);
                clientSocket->deleteLater();
            }
        );

        //on data received from client
        QObject::connect(
            clientSocket, &QIODevice::readyRead, 
            [&, clientSocket]() {
                this->_handleIncomingMessages(clientSocket);
            }
        );

        //signals new connection
        auto newIp = clientSocket->peerAddress().toString();
        emit newConnectionReceived(newIp.toStdString());
        qDebug() << "Chat Server : New connection from " << newIp;

        this->_sendStoredMessages(clientSocket);

}