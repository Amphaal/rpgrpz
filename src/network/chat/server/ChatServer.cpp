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


void ChatServer::start() { 
    this->_server = new QTcpServer;

    qDebug() << "Chat Server : Starting server...";

    auto result = this->_server->listen(QHostAddress::Any, std::stoi(UPNP_DEFAULT_TARGET_PORT));

    if(!result) {
        qWarning() << "Chat Server : Error while starting to listen >> " + this->_server->errorString();
        return;
    } else {
        qDebug() << "Chat Server : Succesfully listening !";
    }

    while (!_isStopped()) {

        bool connectionAvailable = this->_server->waitForNewConnection();
        if (!connectionAvailable) continue; //no connection, rewind

        //new connection,store it
        QTcpSocket *clientConnection = this->_server->nextPendingConnection();
        auto newIp = clientConnection->peerAddress().toString();

        //signals new connection
        emit newConnectionReceived(newIp.toStdString());
        qDebug() << "Chat Server : New connection from " << newIp;

        //auto remove socket if disconnected
        QObject::connect(clientConnection, &QAbstractSocket::disconnected,
                clientConnection, &QObject::deleteLater);

        //on data received from client
        QObject::connect(clientConnection, &QIODevice::readyRead, [&, clientConnection]() {
            auto qq = true;
        });

        _sendWelcomeMessage(clientConnection);
    }

    // Self-destruct the server
    deleteLater();
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
    auto i = true;
}