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
    const QString msg = "Déconnecté du serveur";
    emit error(msg);
    qDebug() << "RPZClient : " << msg;
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
    this->socket()->connectToHost(this->_domain, this->_port.toInt());
    
}

RPZUser RPZClient::identity() {
    return this->_self;
}

QVector<RPZUser> RPZClient::sessionUsers() {
    return this->_sessionUsers;
}

void RPZClient::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {
    
    switch(method) {
        case JSONMethod::ChatLogHistory:
            emit receivedLogHistory(data.toList());
            break;
        case JSONMethod::LoggedPlayersChanged: {
                auto users = data.toHash();

                //store users
                this->_sessionUsers.clear();
                for(auto &rUser : users) this->_sessionUsers.append(RPZUser(rUser.toHash()));

                emit loggedUsersUpdated(users);
            }
            break;
        case JSONMethod::AckIdentity: {
                auto hash = data.toHash();
                
                //store our identity
                this->_self = RPZUser(hash);

                emit ackIdentity(hash);
            }
            break;
        case JSONMethod::AskForHostMapHistory: {
                emit beenAskedForMapHistory();
            }
            break;
        case JSONMethod::MessageFromPlayer: {
                const auto mfp = data.toHash();
                emit receivedMessage(mfp);
            }
            break;
        case JSONMethod::MapChanged: {
                emit mapChanged(data.toHash());
            }   
            break;
        case JSONMethod::RequestedAsset: {
                emit receivedAsset(data.toHash());
            }
            break;
        default:
            qWarning() << "RPZClient : unknown method from JSON !";
    }
}

void RPZClient::informAssetSucessfulInsertion(const QString &assetId) {
    emit assetSucessfullyInserted(assetId);
}

void RPZClient::_error(QAbstractSocket::SocketError _socketError) {
    
    QString msg;
    
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
            msg = "L'erreur suivante s'est produite : " + this->socket()->errorString();
                                   
    }

    emit error(msg);
    qWarning() << "RPZClient : :" << msg;

    this->socket()->close();
}


void RPZClient::sendMessage(QVariantHash &message) {

    auto msg = RPZMessage(message);

    this->sendJSON(JSONMethod::MessageFromPlayer, msg);

    qDebug() << "RPZClient : message sent " << msg.message(); 
}

void RPZClient::askForAsset(const QString &assetId) {

    this->sendJSON(JSONMethod::AskForAsset, assetId);

    qDebug() << "RPZClient : requesting asset " << assetId << " to server"; 
}

void RPZClient::sendMapChanges(QVariantHash &payload) {
    this->sendJSON(JSONMethod::MapChanged, payload);
}
