#include "RPZClient.h"

RPZClient::RPZClient(const QString &name, const QString &domain, const QString &port) : 
                        _name(name), 
                        _domain(domain), 
                        _port(port) { }

RPZClient::~RPZClient() {
    delete this->_cli;
    delete this->_ack;
}

void RPZClient::run() {

    //prerequisites
    if(this->_name.isEmpty()) {
        emit connectionStatus("Nom de joueur requis !", true);
        emit closed();
    }

    this->_cli = new JSONSocket("RPZClient");
    this->_ack = new AlterationAcknoledger(AlterationPayload::Source::RPZClient, false);

    ///
    /// EVENTS
    ///

        QObject::connect(
            AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
            this, &RPZClient::_handleAlterationRequest
        );

        QObject::connect(
            this->_cli, &JSONSocket::JSONReceived,
            this, &RPZClient::_routeIncomingJSON
        );

        QObject::connect(
            this->_cli->socket(), &QAbstractSocket::connected,
            this, &RPZClient::_onConnected
        );

        QObject::connect(
            this->_cli->socket(), &QAbstractSocket::disconnected,
            this, &RPZClient::_onDisconnect
        );

        QObject::connect(
            this->_cli->socket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &RPZClient::_error
        );

    ///
    /// END EVENTS
    ///

    //connect...
    this->_cli->socket()->connectToHost(this->_domain, this->_port.toInt());

}

void RPZClient::_onDisconnect() {
    const QString msg = "Déconnecté du serveur";
    emit connectionStatus(msg);
    qDebug() << "RPZClient : disconnected from server";
}

void RPZClient::_onConnected() {
    //tell the server your username
    this->_cli->sendJSON(JSONMethod::Handshake, RPZHandshake(this->_name));
}

void RPZClient::_handleAlterationRequest(AlterationPayload &payload) {

    //ignore alteration requests when socket is not connected
    if(this->_cli->socket()->state() != QAbstractSocket::ConnectedState) return;

    //if not routable, instant return 
    if(!payload.isNetworkRoutable()) return;

    //send json
    return this->_cli->sendJSON(JSONMethod::MapChanged, payload);
}

QString RPZClient::getConnectedSocketAddress() {
    return this->_domain + ":" + this->_port;
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
            this->_cli->socket()->disconnectFromHost();
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
            auto payload = AlterationPayload(data.toHash());
            AlterationHandler::get()->queueAlteration(this->_ack, payload);
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
            msg = "L'erreur suivante s'est produite : " + this->_cli->socket()->errorString();
                                   
    }

    emit connectionStatus(msg, true);
    qWarning() << "RPZClient : :" << msg;

    emit closed();
}


void RPZClient::sendMessage(QVariantHash &message) {
    auto msg = RPZMessage(message);
    this->_cli->sendJSON(JSONMethod::MessageFromPlayer, msg);
}

void RPZClient::sendMapHistory(const QVariantHash &history) {
    this->_cli->sendJSON(JSONMethod::MapChanged, history);
}

void RPZClient::askForAssets(const QList<RPZAssetHash> ids) {
    QVariantList list;
    for(auto &id : ids) list.append(id);
    this->_cli->sendJSON(JSONMethod::AskForAssets, list);
}

void RPZClient::changeAudioPosition(int newPosition) {
    this->_cli->sendJSON(JSONMethod::AudioStreamPositionChanged, newPosition);
}

void RPZClient::setAudioStreamPlayState(bool isPlaying) {
    this->_cli->sendJSON(JSONMethod::AudioStreamPlayingStateChanged, isPlaying);
}

void RPZClient::defineAudioStreamSource(const QString &audioStreamUrl, const QString &sourceTitle) {
    QVariantHash hash;
    hash["url"] = audioStreamUrl;
    hash["title"] = sourceTitle;
    this->_cli->sendJSON(JSONMethod::AudioStreamUrlChanged, hash);
}