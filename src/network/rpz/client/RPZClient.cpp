#include "RPZClient.h"

RPZClient::RPZClient(QObject* parent, const QString &name, const QString &domain, const QString &port) : 
                        JSONSocket(parent, "RPZClient"),
                        _name(name), 
                        _domain(domain), 
                        _port(port) {

    qDebug() << "RPZClient : Instantiation...";

    QObject::connect(
        this, &JSONSocket::JSONReceived,
        this, &RPZClient::_routeIncomingJSON
    );

    QObject::connect(
        this->socket(), &QAbstractSocket::connected,
        this, &RPZClient::_onConnected
    );

    QObject::connect(
        this->socket(), &QAbstractSocket::disconnected,
        this, &RPZClient::_onDisconnect
    );


    QObject::connect(
        this->socket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        this, &RPZClient::_error
    );

}

RPZClient::~RPZClient() {
    this->socket()->close();
}

void RPZClient::_onDisconnect() {
    const std::string msg = "Déconnecté du serveur";
    emit error(msg);
    qWarning() << "RPZClient : " << QString::fromStdString(msg);
}

void RPZClient::_onConnected() {
    //tell the server your username
    this->sendJSON(JSONMethod::PlayerHasUsername, QStringList(this->_name));
}


QString RPZClient::getConnectedSocketAddress() {
    return this->_domain + ":" + this->_port;
}

void RPZClient::run() {

    //prerequisites
    if(this->_name.isEmpty()) {
        emit error("Nom de joueur requis !");
        return;
    }

    qDebug() << "RPZClient : Connecting...";    
    this->socket()->abort();
    this->socket()->connectToHost(this->_domain, this->_port.toInt());
    
}

void RPZClient::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {
    
    switch(method) {
        case JSONMethod::ChatLogHistory:
            emit receivedLogHistory(data.toList());
            break;
        case JSONMethod::LoggedPlayersChanged: {
                const auto users = data.toHash();
                emit loggedUsersUpdated(users);
            }
            break;
        case JSONMethod::AskForHostMapHistory: {
                emit beenAskedForMapHistory();
            }
            break;
        case JSONMethod::MessageFromPlayer: {
                const auto mfp = data.toList()[0].toString().toStdString();
                emit receivedMessage(mfp);
            }
            break;
        case JSONMethod::HostMapChanged: {
                emit hostMapChanged(data.toList());
            }   
            break;
        default:
            qWarning() << "RPZClient : unknown method from JSON !";
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
            msg = "L'erreur suivante s'est produite : " + this->socket()->errorString().toStdString();
                                   
    }

    emit error(msg);
    qWarning() << "RPZClient : :" << QString::fromStdString(msg);

    this->socket()->close();
}


void RPZClient::sendMessage(const QString &messageToSend) {

    this->sendJSON(JSONMethod::MessageFromPlayer, QStringList(messageToSend));

    qDebug() << "RPZClient : message sent " << messageToSend; 
}


void RPZClient::sendMapHistory(const QVariantList &history) {
    this->sendJSON(JSONMethod::HostMapHistory, history);

    qDebug() << "RPZClient : map history sent"; 
}
