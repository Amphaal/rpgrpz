#include "RPZClient.h"

RPZClient::RPZClient(const QString &name, const QString &domain, const QString &port) : 
                        JSONSocket("RPZClient"),
                        AlterationAcknoledger(AlterationPayload::Source::RPZClient, false),
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

RPZClient::~RPZClient() { }

void RPZClient::run() {

    //prerequisites
    if(this->_name.isEmpty()) {
        emit connectionStatus("Nom de joueur requis !", true);
        emit closed();
    }

    //connect...
    this->socket()->connectToHost(this->_domain, this->_port.toInt());

}

void RPZClient::_onDisconnect() {
    const QString msg = "Déconnecté du serveur";
    emit connectionStatus(msg);
    qDebug() << "RPZClient : disconnected from server";
}

void RPZClient::_onConnected() {
    //tell the server your username
    this->sendJSON(JSONMethod::Handshake, RPZHandshake(this->_name));
}

void RPZClient::_handleAlterationRequest(AlterationPayload &payload) {

    //ignore alteration requests when socket is not connected
    if(this->socket()->state() != QAbstractSocket::ConnectedState) return;

    //if not routable, instant return 
    if(!payload.isNetworkRoutable()) return;

    //send json
    return this->sendJSON(JSONMethod::MapChanged, payload);
}

QString RPZClient::getConnectedSocketAddress() const {
    return this->_domain + ":" + this->_port;
}

RPZUser RPZClient::identity() const {
    QMutexLocker l(&this->_m_self);
    return this->_self;
}

QVector<RPZUser> RPZClient::sessionUsers() const {
    QMutexLocker l(&this->_m_sessionUsers);
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
            QVector<RPZMessage> msgs;
            for(auto &rawMsg : data.toList()) {
                RPZMessage msg(rawMsg.toHash());
                msgs.append(msg);
            }
            emit receivedLogHistory(msgs);
        }
        break;

        case JSONMethod::LoggedPlayersChanged: {
            
            {
                //store users
                QMutexLocker l(&this->_m_sessionUsers);
                this->_sessionUsers.clear();
                
                for(auto &rUser : data.toList()) {
                    RPZUser user(rUser.toHash());
                    this->_sessionUsers.append(user);
                }
            }

            emit loggedUsersUpdated(this->_sessionUsers);
        }
        break;

        case JSONMethod::AckIdentity: {
            
            {
                //store our identity
                QMutexLocker l(&this->_m_self);
                this->_self = RPZUser(data.toHash());
            }

            emit ackIdentity(this->_self);

        }
        break;

        case JSONMethod::AskForHostMapHistory: {
            emit beenAskedForMapHistory();
        }
        break;

        case JSONMethod::MessageFromPlayer: {
            RPZMessage msg(data.toHash());
            emit receivedMessage(msg);
        }
        break;

        case JSONMethod::MapChanged: {
            auto payload = AlterationPayload(data.toHash());
            AlterationHandler::get()->queueAlteration(this, payload);
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
            msg = "L'erreur suivante s'est produite : " + this->socket()->errorString();
                                   
    }

    emit connectionStatus(msg, true);
    qWarning() << "RPZClient : :" << msg;

    emit closed();
}


void RPZClient::sendMessage(const RPZMessage &message) {
    auto msg = RPZMessage(message);
    this->sendJSON(JSONMethod::MessageFromPlayer, msg);
}

void RPZClient::sendMapHistory(const ResetPayload &historyPayload) {
    this->sendJSON(JSONMethod::MapChanged, historyPayload);
}

void RPZClient::askForAssets(const QList<RPZAssetHash> ids) {
    QVariantList list;
    for(auto &id : ids) list.append(id);
    this->sendJSON(JSONMethod::AskForAssets, list);
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