#include "ChatClient.h"

ChatClient::ChatClient(QString name, QString domain, QString port) : 
                        _name(name), 
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
}

void ChatClient::close() {
    this->_socket->close();
}

void ChatClient::sendMessage(QString messageToSend) {
    //prepare
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out.setDevice(this->_socket);

    //message...
    out << formatChatMessage(this->_name, messageToSend);
    
    //write
    auto written = this->_socket->write(block);
    auto i = this->_socket->waitForBytesWritten();

    qDebug() << "Chat Client : message sent >> " << messageToSend << "<<";
}

void ChatClient::tryConnection() {
    
    auto pet = this->_name.toStdString();

    //prerequisites
    if(this->_name.isEmpty()) {
        emit error("Nom de joueur requis !");
        return;
    }

    qDebug() << "Chat Client : Connecting...";    
    this->_socket->abort();
    this->_socket->connectToHost(this->_domain, this->_port.toInt());
}

//receiving data...
void ChatClient::_onRR() {

        _in.startTransaction();

        QByteArray block;
        _in >> block;

        if (!_in.commitTransaction()) {
            qWarning() << "Chat Client : issue while reading incoming message";  
            return;
        }
        
        _JSONTriage(block);

}

void ChatClient::_JSONTriage(QByteArray &potentialJSON) {

    auto json = QJsonDocument::fromBinaryData(potentialJSON);
    
    if(json.isNull()) {
        qWarning() << "Chat Client : Data received from server was not JSON and thus cannot be read.";
        return;
    }
    
    //triage
    auto content = json.object();
    auto mainKeys = content.keys();
    
    if(mainKeys.contains("messages_history")) {
       auto msg_list = content["messages_history"].toArray().toVariantList();
       for(auto msg : msg_list) {
           auto stdmsg = msg.toString().toStdString();
           emit receivedMessage(stdmsg);
       }
       emit historyReceived();
    } else if (mainKeys.contains("new_message")) {
        auto stdmsg = content["new_message"].toString().toStdString();
        emit receivedMessage(stdmsg);
    } else {
        qDebug() << "Chat Client : Data received from server >>" << json;
    }

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