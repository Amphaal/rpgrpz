#include "RPZClientThread.h"

RPZClientThread::RPZClientThread(QObject* parent, const QString &name, const QString &domain, const QString &port) : 
                        AtomAlterationAcknoledger(AlterationPayload::Source::RPZClient),
                        JSONSocket("RPZClientThread"),
                        _name(name), 
                        _domain(domain), 
                        _port(port) {

    QObject::connect(
        this, &JSONSocket::JSONReceived,
        this, &RPZClientThread::_routeIncomingJSON
    );

    QObject::connect(
        this->socket(), &QAbstractSocket::connected,
        this, &RPZClientThread::_onConnected
    );

    QObject::connect(
        this->socket(), &QAbstractSocket::disconnected,
        this, &RPZClientThread::_onDisconnect
    );

    QObject::connect(
        this->socket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        this, &RPZClientThread::_error
    );

}

RPZClientThread::~RPZClientThread() {
    this->socket()->close();
}

void RPZClientThread::_onDisconnect() {
    const QString msg = "Déconnecté du serveur";
    emit connectionStatus(msg);
    qDebug() << "RPZClientThread : disconnected from server";
}

void RPZClientThread::_onConnected() {
    //tell the server your username
    this->sendJSON(JSONMethod::Handshake, RPZHandshake(this->_name));
}

QFuture<void> RPZClientThread::_handleAlterationRequest(AlterationPayload &payload, bool autoPropagate) {
    
    //completed deferred
    auto d = AsyncFuture::deferred<void>();
    d.complete();

    //ignore alteration requests when socket is not connected
    if(this->socket()->state() != QAbstractSocket::ConnectedState) return d.future();

    //trace
    this->_payloadTrace(payload);

    //if not routable, instant return 
    if(!payload.isNetworkRoutable()) return d.future();

    //send json
    this->sendJSON(JSONMethod::MapChanged, payload);
    return d.future();
}

QString RPZClientThread::getConnectedSocketAddress() {
    return this->_domain + ":" + this->_port;
}

void RPZClientThread::run() {

    //prerequisites
    if(this->_name.isEmpty()) {
        emit connectionStatus("Nom de joueur requis !", true);
        return;
    }

    this->socket()->connectToHost(this->_domain, this->_port.toInt());
}

RPZUser RPZClientThread::identity() {
    return this->_self;
}

QVector<RPZUser> RPZClientThread::sessionUsers() {
    return this->_sessionUsers;
}

void RPZClientThread::_routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) {
    
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
            auto payload = Payloads::autoCast(data.toHash());
            this->propagateAlterationPayload(*payload);
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

void RPZClientThread::_error(QAbstractSocket::SocketError _socketError) {
    
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
    qWarning() << "RPZClientThread : :" << msg;

    this->socket()->close();
}


void RPZClientThread::sendMessage(QVariantHash &message) {
    auto msg = RPZMessage(message);
    this->sendJSON(JSONMethod::MessageFromPlayer, msg);
}

void RPZClientThread::sendMapHistory(const QVariantHash &history) {
    this->sendJSON(JSONMethod::MapChanged, history);
}

void RPZClientThread::askForAssets(const QList<RPZAssetHash> ids) {
    QVariantList list;
    for(auto &id : ids) list.append(id);
    this->sendJSON(JSONMethod::AskForAssets, list);
}

void RPZClientThread::changeAudioPosition(int newPosition) {
    this->sendJSON(JSONMethod::AudioStreamPositionChanged, newPosition);
}

void RPZClientThread::setAudioStreamPlayState(bool isPlaying) {
    this->sendJSON(JSONMethod::AudioStreamPlayingStateChanged, isPlaying);
}

void RPZClientThread::defineAudioStreamSource(const QString &audioStreamUrl, const QString &sourceTitle) {
    QVariantHash hash;
    hash["url"] = audioStreamUrl;
    hash["title"] = sourceTitle;
    this->sendJSON(JSONMethod::AudioStreamUrlChanged, hash);
}