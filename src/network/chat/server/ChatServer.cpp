#include "ChatServer.h" 

ChatServer::ChatServer() { };

ChatServer::~ChatServer() {
    qDebug() << "Chat Server : Server ending !";

    this->_server->close();
};


bool ChatServer::isStopped() {
    bool stopped;
    mutex.lock();
    stopped = this->stopped;
    mutex.unlock();
    return stopped;
}

void ChatServer::stop() {
    mutex.lock();
    stopped = true;
    mutex.unlock();
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

    while (!isStopped()) {

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

        //send welcome message
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_12);
        out << "caca";
        auto written = clientConnection->write(block);
        clientConnection->disconnectFromHost();
    }

    // Self-destruct the server
    deleteLater();
};

