#include "ChatServer.h" 

ChatServer::ChatServer() { };

ChatServer::~ChatServer() {
    qDebug() << "Chat Server : Server ending !";

    this->_server->close();
};


bool ChatServer::_isStopped() {
    bool _stopped;
    _mutex.lock();
    _stopped = this->_stopped;
    _mutex.unlock();
    return _stopped;
}

void ChatServer::stop() {
    _mutex.lock();
    _stopped = true;
    _mutex.unlock();
}


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

};

void ChatServer::_sendWelcomeMessage(QTcpSocket* clientSocket) {
    
    //send welcome message
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    
    //message...
    out << this->_messages;
    auto written = clientSocket->write(block);

    //wait end send
    clientSocket->waitForBytesWritten();

    qDebug() << "Chat Server : stored messages sent to " << clientSocket->peerAddress().toString();
}

void ChatServer::_handleIncomingMessages(QTcpSocket * clientSocket) {

    QDataStream in;
    in.setVersion(QDataStream::Qt_5_12);
    in.setDevice(clientSocket);

    in.startTransaction();

    QString result; 
    in >> result;
    this->_messages << result;

    if (!in.commitTransaction()) {
        qWarning() << "Chat Server : issue while reading incoming message";  
        return;
    }

    qDebug() << "Chat Server : message received from " << clientSocket->peerAddress().toString() << " >> " << result;
}

void ChatServer::_onNewConnection() {
        
        //new connection,store it
        auto clientSocket = this->_server->nextPendingConnection();
        
        //auto remove socket if disconnected
        QObject::connect(
            clientSocket, &QAbstractSocket::disconnected,
            clientSocket, &QObject::deleteLater
        );

        //on data received from client
        QObject::connect(
            clientSocket, &QIODevice::readyRead, 
            [&, clientSocket]() {
                this->_handleIncomingMessages(clientSocket);
            }
        );
        auto newIp = clientSocket->peerAddress().toString();

        //signals new connection
        emit newConnectionReceived(newIp.toStdString());
        qDebug() << "Chat Server : New connection from " << newIp;

        this->_sendWelcomeMessage(clientSocket);

        this->_clientSockets << clientSocket;
}