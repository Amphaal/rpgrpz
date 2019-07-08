#include "RPZClient.h"

RPZClient::RPZClient(QObject* parent, const QString &name, const QString &domain, const QString &port) : 
                        JSONSocket(parent, "RPZClient"),
                        _name(name), 
                        _domain(domain), 
                        _port(port) {

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
    emit connectionStatus(msg);
    qDebug() << "RPZClient : " << msg;
}

void RPZClient::_onConnected() {
    //tell the server your username
    this->sendJSON(JSONMethod::Handshake, RPZHandshake(this->_name));
}


QString RPZClient::getConnectedSocketAddress() {
    return this->_domain + ":" + this->_port;
}

void RPZClient::run() {

    //prerequisites
    if(this->_name.isEmpty()) {
        emit connectionStatus("Nom de joueur requis !", true);
        return;
    }

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

        case JSONMethod::ServerStatus: {
            emit connectionStatus(data.toString(), true);
            this->socket()->disconnectFromHost();
        }
        break;

        case JSONMethod::AudioStreamPositionChanged: {
            auto newPos = data.toInt();
            emit audioPositionChanged(newPos);
        }
        break;

        case JSONMethod::AudioStreamPlayingStateChanged: {
            auto isPlaying = data.toBool();
            emit audioPlayStateChanged(isPlaying);
        }
        break;

        case JSONMethod::AudioStreamUrlChanged: {
            auto payload = data.toHash();
            emit audioSourceChanged(payload["url"].toString(), payload["title"].toString());
        }
        break;
        
        case JSONMethod::ChatLogHistory: {
            emit receivedLogHistory(data.toList());
        }
        break;

        case JSONMethod::LoggedPlayersChanged: {
            auto users = data.toList();

            //store users
            this->_sessionUsers.clear();
            for(auto &rUser : users) {
                RPZUser user(rUser.toHash());
                this->_sessionUsers.append(user);
            }

            emit loggedUsersUpdated(users);
        }
        break;

        case JSONMethod::AckIdentity: {
            
            //store our identity
            this->_self = RPZUser(data.toHash());

            emit ackIdentity(this->_self);
        }
        break;

        case JSONMethod::AskForHostMapHistory: {
            emit beenAskedForMapHistory();
        }
        break;

        case JSONMethod::MessageFromPlayer: {
            emit receivedMessage(data.toHash());
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

        case JSONMethod::ServerResponse: {
            emit serverResponseReceived(data.toHash());
        }
        break;

        default:
            break;
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

    emit connectionStatus(msg, true);
    qWarning() << "RPZClient : :" << msg;

    this->socket()->close();
}


void RPZClient::sendMessage(QVariantHash &message) {
    auto msg = RPZMessage(message);
    this->sendJSON(JSONMethod::MessageFromPlayer, msg);
}

void RPZClient::askForAsset(const QString &assetId) {
    this->sendJSON(JSONMethod::AskForAsset, assetId);
}

void RPZClient::sendMapChanges(QVariantHash &payload) {
    this->sendJSON(JSONMethod::MapChanged, payload);
}

void RPZClient::changeAudioPosition(int newPosition) {
    this->sendJSON(JSONMethod::AudioStreamPositionChanged, newPosition);
}

void RPZClient::setAudioStreamPlayState(bool isPlaying) {
    this->sendJSON(JSONMethod::AudioStreamPlayingStateChanged, isPlaying);
}

void RPZClient::defineAudioStreamSource(const QString &audioStreamUrl, const QString &sourceTitle) {
    QVariantHash hash;
    hash["url"] = audioStreamUrl;
    hash["title"] = sourceTitle;
    this->sendJSON(JSONMethod::AudioStreamUrlChanged, hash);
}