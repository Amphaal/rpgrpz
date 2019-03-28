#include "ChatServer.h" 

ChatServer::ChatServer() : _server(new QTcpServer) { 
    
    this->_server->moveToThread(this);

    qDebug() << "Chat Server : Instanciating...";

    QObject::connect(
        this->_server, &QTcpServer::newConnection,
        this, &ChatServer::_onNewConnection
    );

    QObject::connect(
        this, &QThread::finished, 
        this, &ChatServer::_onFinished
    );
};


void ChatServer::run() { 
    
    qDebug() << "Chat Server : Starting server...";

    auto result = this->_server->listen(QHostAddress::Any, std::stoi(UPNP_DEFAULT_TARGET_PORT));

    if(!result) {
        qWarning() << "Chat Server : Error while starting to listen >> " + this->_server->errorString();
        return;
    } else {
        qDebug() << "Chat Server : Succesfully listening !";
    }

    this->exec();

};

void ChatServer::_onFinished() {
    qDebug() << "Chat Server : Server Thread finished !";

    this->_server->close();
};

void ChatServer::_onNewConnection() {
    
    qDebug() << "Chat Server : New connection !";

    //store connection
    QTcpSocket *clientConnection = this->_server->nextPendingConnection();
    QObject::connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);
    _clients.append(clientConnection);

    //send welcome message
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << "caca";
    clientConnection->write(block);
    //clientConnection->disconnectFromHost();
};