#include "RPZClient.h"

RPZClient::RPZClient(const QString &socketStr, const QString &displayName, const RPZCharacter &toIncarnate) : AlterationActor(AlterationPayload::Source::RPZClient), 
    _userDisplayName(displayName),
    _characterToIncarnate(toIncarnate) { 
    
    //split socket str
    auto parts = socketStr.split(":", QString::SplitBehavior::SkipEmptyParts);
    this->_domain = parts.value(0, "localhost");
    this->_port = parts.value(1, AppContext::UPNP_DEFAULT_TARGET_PORT);

}

void RPZClient::_initSock() {
    this->_sock = new JSONSocket(this, "RPZClient");

    QObject::connect(
        this->_sock, &JSONSocket::JSONReceived,
        this, &RPZClient::_routeIncomingJSON
    );

    QObject::connect(
        this->_sock->socket(), &QAbstractSocket::connected,
        this, &RPZClient::_onConnected
    );

    QObject::connect(
        this->_sock, &JSONSocket::sending,
        this, &RPZClient::_onSending
    );

    QObject::connect(
        this->_sock, &JSONSocket::sent,
        this, &RPZClient::_onSent
    );

    QObject::connect(
        this->_sock, &JSONSocket::ackedBatch,
        this, &RPZClient::_onBatchAcked
    );

    QObject::connect(
        this->_sock, &JSONSocket::batchDownloading,
        this, &RPZClient::_onBatchDownloading
    );

    QObject::connect(
        this->_sock->socket(), &QAbstractSocket::disconnected,
        this, &RPZClient::_onDisconnect
    );

    QObject::connect(
        this->_sock->socket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        this, &RPZClient::_error
    );

    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &RPZClient::_handleAlterationRequest
    );
}

void RPZClient::_onBatchAcked(JSONMethod method, qint64 batchSize) {
    
    if(method != JSONMethod::MapChangedHeavily) return;

    QMetaObject::invokeMethod(ProgressTracker::get(), "downloadIsStarting", 
        Q_ARG(ProgressTracker::Kind, ProgressTracker::Kind::Map), 
        Q_ARG(qint64, batchSize)
    );

}

void RPZClient::_onBatchDownloading(JSONMethod method, qint64 downloaded) {
    
    if(method != JSONMethod::MapChangedHeavily) return;

    QMetaObject::invokeMethod(ProgressTracker::get(), "downloadIsProgressing", 
        Q_ARG(ProgressTracker::Kind, ProgressTracker::Kind::Map), 
        Q_ARG(qint64, downloaded)
    );

}

RPZClient::~RPZClient() { 
    if(this->_sock) delete this->_sock;
}

void RPZClient::run() {

    //prerequisites
    if(this->_userDisplayName.isEmpty()) {
        emit connectionStatus("Nom de joueur requis !", true);
        emit closed();
    }

    this->_initSock();

    //connect...
    this->_sock->socket()->connectToHost(this->_domain, this->_port.toInt());

}

void RPZClient::_onDisconnect() {
    const QString msg = "Déconnecté du serveur";
    emit connectionStatus(msg);
    qDebug() << "RPZClient : disconnected from server";
}

void RPZClient::_onConnected() {
    
    //tell the server your username
    this->_sock->sendJSON(JSONMethod::Handshake, 
        RPZHandshake(
            this->_userDisplayName,
            this->_characterToIncarnate
        )
    );

}

void RPZClient::_handleAlterationRequest(const AlterationPayload &payload) {

    //ignore packages from server
    if(payload.source() == this->source()) return;
    if(payload.source() == AlterationPayload::Source::RPZServer) return;

    //ignore alteration requests when socket is not connected
    if(this->_sock->socket()->state() != QAbstractSocket::ConnectedState) return;

    //if not routable, instant return 
    if(!payload.isNetworkRoutable()) return;

    this->payloadTrace(payload);

    //send json
    auto method = payload.type() == PA_Reset ? JSONMethod::MapChangedHeavily : JSONMethod::MapChanged;
    return this->_sock->sendJSON(method, payload);
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
    
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientIsReceiving");

    switch(method) {

        case JSONMethod::AvailableAssetsToUpload: {
            
            //cast
            QVector<RPZAssetHash> out;
            for(auto &e : data.toList()) out += e.toString();

            //update ui
            QMetaObject::invokeMethod(ProgressTracker::get(), "downloadIsStarting", 
                Q_ARG(ProgressTracker::Kind, ProgressTracker::Kind::Asset), 
                Q_ARG(qint64, out.count())
            );

            //emit
            emit availableAssetsFromServer(out);
            
        }
        break;

        case JSONMethod::ServerStatus: {
            emit connectionStatus(data.toString(), true);
            this->_sock->socket()->disconnectFromHost();
        }
        break;

        case JSONMethod::AudioStreamPositionChanged: {
            auto newPos = data.value<qint64>();
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
            emit audioSourceStateChanged(payload);
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

        case JSONMethod::AllConnectedUsers: {
            
            {
                //store users
                QMutexLocker l(&this->_m_sessionUsers);
                this->_sessionUsers.clear();
                
                for(auto &rUser : data.toList()) {
                    RPZUser user(rUser.toHash());
                    this->_sessionUsers.append(user);
                }
            }

            emit allUsersReceived(this->_sessionUsers);
        }
        break;

        case JSONMethod::UserIn: {
            emit userJoinedServer(RPZUser(data.toHash()));
        }
        break;

        case JSONMethod::UserOut: {
            emit userLeftServer(data.toULongLong());
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

        case JSONMethod::Message: {
            RPZMessage msg(data.toHash());
            emit receivedMessage(msg);
        }
        break;

        case JSONMethod::MapChangedHeavily:
        case JSONMethod::MapChanged: {
            
            //to sharedPointer for type casts
            auto payload = Payloads::autoCast(data.toHash());

            //if atom weilder, check if assets are missing
            if(auto mPayload = dynamic_cast<AtomsWielderPayload*>(payload.data())) { 
                
                //compare assets in map with assets in db 
                auto missingAssetIds = mPayload->assetIds();
                auto handledMissingAssetIds = AssetsDatabase::get()->getStoredAssetsIds();
                missingAssetIds.subtract(handledMissingAssetIds);
                
                //if missing assets, request them
                if(auto count = missingAssetIds.count()) {
                    qDebug() << "Assets : missing" << QString::number(count).toStdString().c_str() << "asset(s)";
                    this->_askForAssets(missingAssetIds);
                }

            }

            AlterationHandler::get()->queueAlteration(this, *payload);
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

    QMetaObject::invokeMethod(ProgressTracker::get(), "clientStoppedReceiving");
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
            msg = "L'erreur suivante s'est produite : " + this->_sock->socket()->errorString();
                                   
    }

    emit connectionStatus(msg, true);
    qDebug() << "RPZClient : :" << msg.toStdString().c_str();

    emit closed();
}


void RPZClient::sendMessage(const RPZMessage &message) {
    auto msg = RPZMessage(message);
    this->_sock->sendJSON(JSONMethod::Message, msg);
}

void RPZClient::sendMapHistory(const ResetPayload &historyPayload) {
    this->_sock->sendJSON(JSONMethod::MapChangedHeavily, historyPayload);
}

void RPZClient::_askForAssets(const QSet<RPZAssetHash> &ids) {
    QVariantList list;
    for(auto &id : ids) list.append(id);
    this->_sock->sendJSON(JSONMethod::AskForAssets, list);
}

void RPZClient::changeAudioPosition(qint64 newPositionInMsecs) {
    this->_sock->sendJSON(JSONMethod::AudioStreamPositionChanged, newPositionInMsecs);
}

void RPZClient::setAudioStreamPlayState(bool isPlaying) {
    this->_sock->sendJSON(JSONMethod::AudioStreamPlayingStateChanged, isPlaying);
}

void RPZClient::defineAudioSourceState(const StreamPlayStateTracker &state) {
    this->_sock->sendJSON(JSONMethod::AudioStreamUrlChanged, state);
}

void RPZClient::_onSending() {
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientIsSending");
}

void RPZClient::_onSent() {
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientStoppedSending");
}