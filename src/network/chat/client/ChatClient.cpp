#include "ChatClient.h"

ChatClient::ChatClient(QString displayname, QString domain, QString port) : 
                        _dn(displayname), 
                        _domain(domain), 
                        _port(port), 
                        _socket(new QTcpSocket) {
    
    qDebug() << "Chat Client : Instantiation...";
    
    this->_in.setVersion(QDataStream::Qt_5_12);
    this->_in.setDevice(this->_socket);

    QObject::connect(
        this->_socket, &QIODevice::readyRead, 
        this, &ChatClient::_onRR
    );

    QObject::connect(
        this->_socket, &QAbstractSocket::disconnected,
        [&]() {
            std::string msg = "Déconnecté du serveur";
            emit error(msg);
            qWarning() << "Chat Client : :" << QString::fromStdString(msg);
        }
    );

    QObject::connect(
        this->_socket, &QAbstractSocket::connected,
        [&]() {
            emit connected();
        }
    );

    QObject::connect(
        this->_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        this, &ChatClient::_error
    );

    this->_tryConnection();
}

void ChatClient::close() {
    this->_socket->close();
}

void ChatClient::sendMessage(QString messageToSend) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out.setDevice(this->_socket);

    //message...
    out << messageToSend;
    auto written = this->_socket->write(block);
    
    auto i = this->_socket->waitForBytesWritten();

    qDebug() << "Chat Client : message sent >> " << messageToSend << "<<";
}

void ChatClient::_tryConnection() {
    qDebug() << "Chat Client : Connecting...";    
    this->_socket->abort();
    this->_socket->connectToHost(this->_domain, this->_port.toInt());
}

//receiving data...
void ChatClient::_onRR() {

        _in.startTransaction();

        QString result; 
        _in >> result;

        if (!_in.commitTransaction()) {
            qWarning() << "Chat Client : issue while reading incoming message";  
            return;
        }
        
        qDebug() << "Chat Client : Data received from server >>" << result;    
}

void ChatClient::_error(QAbstractSocket::SocketError _socketError) {
    
    std::string msg;
    
    switch (_socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            msg = "L'hôte a fermé la connexion.";
            break;
        case QAbstractSocket::HostNotFoundError:
            msg = "L'hôte n'a pa pu être trouvé. Merci de vérifier le nom / l'IP de l'hôte et le numéro de port.";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            msg = "La connexion a été refusée par l'hôte distant.";
            break;
        default:
            msg = "L'erreur suivante s'est produite : " + this->_socket->errorString().toStdString();
                                   
    }

    emit error(msg);
    qWarning() << "Chat Client : :" << QString::fromStdString(msg);

    this->close();
}

QString ChatClient::getConnectedSocketAddress() {
    return this->_domain + ":" + this->_port;
}