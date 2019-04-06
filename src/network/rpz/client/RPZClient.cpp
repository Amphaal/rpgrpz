#include "RPZClient.h"

RPZClient::RPZClient(QString name, QString domain, QString port) : 
                        _name(name), 
                        _domain(domain), 
                        _port(port) {

    qDebug() << "Chat Client : Instantiation...";

}

void RPZClient::_constructorInThread(){
    
    this->_sockWrapper = new JSONSocket("Chat Client");
    auto qq = this->_sockWrapper->socket();

    QObject::connect(
        this->_sockWrapper, &JSONSocket::JSONReceived,
        [&](JSONSocket* target, JSONMethod method, QVariant data) {
            this->_routeIncomingJSON(target, method, data);
        } 
    );

    QObject::connect(
        this->_sockWrapper->socket(), &QAbstractSocket::disconnected,
        [&]() {
            std::string msg = "Déconnecté du serveur";
            emit error(msg);
            qWarning() << "Chat Client : " << QString::fromStdString(msg);
        }
    );

    QObject::connect(
        this->_sockWrapper->socket(), &QAbstractSocket::connected,
        [&]() {
            
            //tell the server your username
            this->_sockWrapper->sendJSON(JSONMethod::PlayerHasUsername, QStringList(this->_name));

        }
    );

    QObject::connect(
        this->_sockWrapper->socket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        [&](QAbstractSocket::SocketError _socketError) {
            this->_error(_socketError);
        }
    );
}


void RPZClient::sendMessage(QString messageToSend) {

    this->_sockWrapper->sendJSON(JSONMethod::MessageFromPlayer, QStringList(messageToSend));

    qDebug() << "Chat Client : message sent " << messageToSend; 
}

void RPZClient::run() {

    this->_constructorInThread();

    //prerequisites
    if(this->_name.isEmpty()) {
        emit error("Nom de joueur requis !");
        return;
    }

    qDebug() << "Chat Client : Connecting...";    
    this->_sockWrapper->socket()->abort();
    this->_sockWrapper->socket()->connectToHost(this->_domain, this->_port.toInt());
    
    auto isConnected = this->_sockWrapper->socket()->waitForConnected(3000);

    if(isConnected) {
        this->exec();
    } else {
        emit error("Le serveur distant n'a pas pu répondre à temps. Est-t-il disponible ?");
    }

    this->_sockWrapper->socket()->close();
}


void RPZClient::_routeIncomingJSON(JSONSocket* target, JSONMethod method, QVariant data) {
    
    switch(method) {
        case JSONMethod::ChatLogHistory:
            for(auto msg : data.toList()) {
                auto stdmsg = msg.toString().toStdString();
                emit receivedMessage(stdmsg);
            }
            emit logHistoryReceived();
            break;
        case JSONMethod::LoggedPlayersChanged: {
                auto users = data.toList();
                emit loggedUsersUpdated(users);
            }
            break;
        case JSONMethod::MessageFromPlayer: {
                auto mfp = data.toList()[0].toString().toStdString();
                emit receivedMessage(mfp);
            }
            break;
        case JSONMethod::HostMapChanged: {
                emit hostMapChanged(data.toList());
            }   
            break;
        default:
            qWarning() << "Chat Client : unknown method from JSON !";
    }
}

void RPZClient::_error(QAbstractSocket::SocketError _socketError) {
    
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
            msg = "L'erreur suivante s'est produite : " + this->_sockWrapper->socket()->errorString().toStdString();
                                   
    }

    emit error(msg);
    qWarning() << "Chat Client : :" << QString::fromStdString(msg);

    this->exit();
}

QString RPZClient::getConnectedSocketAddress() {
    return this->_domain + ":" + this->_port;
}