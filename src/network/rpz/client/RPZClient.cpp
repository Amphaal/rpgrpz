#include "RPZClient.h"

RPZClient::RPZClient(const QString &socketStr, const QString &displayName, const RPZCharacter &toIncarnate) : AlterationActor(Payload::Source::RPZClient), 
    _userDisplayName(displayName),
    _characterToIncarnate(toIncarnate) { 
    
    //split socket str
    auto parts = socketStr.split(QStringLiteral(u":"), QString::SplitBehavior::SkipEmptyParts);
    this->_domain = parts.value(0, QStringLiteral(u"localhost"));
    this->_port = parts.value(1, AppContext::UPNP_DEFAULT_TARGET_PORT);

}

void RPZClient::_initSock() {
    this->_sock = new JSONSocket(this, QStringLiteral(u"RPZClient"));

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

void RPZClient::_onBatchAcked(RPZJSON::Method method, qint64 batchSize) {
    
    if(method != RPZJSON::Method::MapChangedHeavily) return;

    QMetaObject::invokeMethod(ProgressTracker::get(), "downloadIsStarting", 
        Q_ARG(ProgressTracker::Kind, ProgressTracker::Kind::Map), 
        Q_ARG(qint64, batchSize)
    );

}

void RPZClient::_onBatchDownloading(RPZJSON::Method method, qint64 downloaded) {
    
    if(method != RPZJSON::Method::MapChangedHeavily) return;

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
        emit connectionStatus(tr("Username required !"), true);
        emit closed();
    }

    this->_initSock();

    //connect...
    this->_sock->socket()->connectToHost(this->_domain, this->_port.toInt());

}

void RPZClient::_onDisconnect() {
    emit connectionStatus(tr("Disconnected from server"));
    qDebug() << "RPZClient : disconnected from server";
}

void RPZClient::_onConnected() {
    
    RPZHandshake handshake(
        this->_userDisplayName,
        this->_characterToIncarnate
    );

    //tell the server your username
    this->_sock->sendJSON(
        RPZJSON::Method::Handshake,
        handshake
    );

}

void RPZClient::_handleAlterationRequest(const AlterationPayload &payload) {

    //ignore packages from server
    if(payload.source() == this->source()) return;
    if(payload.source() == Payload::Source::RPZServer) return;

    //prevent alteration propagation to server if not host
    if(this->_self.role() != RPZUser::Role::Host) return;

    //ignore alteration requests when socket is not connected
    if(this->_sock->socket()->state() != QAbstractSocket::ConnectedState) return;

    //if not routable, instant return 
    if(!payload.isNetworkRoutable()) return;

    this->payloadTrace(payload);

    //send json
    auto method = payload.type() == Payload::Alteration::Reset ? RPZJSON::Method::MapChangedHeavily : RPZJSON::Method::MapChanged;
    return this->_sock->sendJSON(method, payload);
}

const QString RPZClient::getConnectedSocketAddress() const {
    return this->_domain + ":" + this->_port;
}

const RPZUser RPZClient::identity() const {
    QMutexLocker l(&this->_m_self);
    return this->_self;
}

const RPZMap<RPZUser> RPZClient::sessionUsers() const {
    QMutexLocker l(&this->_m_sessionUsers);
    return this->_sessionUsers;
}

void RPZClient::_routeIncomingJSON(JSONSocket* target, const RPZJSON::Method &method, const QVariant &data) {
    
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientIsReceiving");

    switch(method) {

        case RPZJSON::Method::AvailableAssetsToUpload: {
            
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

        case RPZJSON::Method::ServerStatus: {
            emit connectionStatus(data.toString(), true);
            this->_sock->socket()->disconnectFromHost();
        }
        break;

        case RPZJSON::Method::AudioStreamPositionChanged: {
            auto newPos = data.value<qint64>();
            emit audioPositionChanged(newPos);
        }
        break;

        case RPZJSON::Method::AudioStreamPlayingStateChanged: {
            auto isPlaying = data.toBool();
            emit audioPlayStateChanged(isPlaying);
        }
        break;

        case RPZJSON::Method::AudioStreamUrlChanged: {
            auto payload = data.toHash();
            emit audioSourceStateChanged(payload);
        }
        break;
        
        case RPZJSON::Method::ChatLogHistory: {
            QVector<RPZMessage> msgs;
            for(auto &rawMsg : data.toList()) {
                RPZMessage msg(rawMsg.toHash());
                msgs.append(msg);
            }
            emit receivedLogHistory(msgs);
        }
        break;

        case RPZJSON::Method::AllConnectedUsers: {
            
            {
                QMutexLocker l(&this->_m_sessionUsers);
                
                //replace all stored users
                this->_sessionUsers.clear();
                for(auto &rUser : data.toList()) {
                    RPZUser user(rUser.toHash());
                    this->_sessionUsers.insert(user.id(), user);
                }

            }

            emit allUsersReceived();
            emit whisperTargetsChanged();
        }
        break;

        case RPZJSON::Method::UserIn: {
            
            RPZUser user(data.toHash());
            
            {
                QMutexLocker l(&this->_m_sessionUsers);
                
                //add user to session users
                this->_sessionUsers.insert(user.id(), user);

            }

            emit userJoinedServer(user);
            emit whisperTargetsChanged();
        }
        break;

        case RPZJSON::Method::UserOut: {
            auto idToRemove = data.toULongLong();

            RPZUser out;

            {
                QMutexLocker l(&this->_m_sessionUsers);

                //remove from session users
                out = this->_sessionUsers.take(idToRemove);

            }

            emit userLeftServer(out);
            emit whisperTargetsChanged();

        }
        break;

        case RPZJSON::Method::UserDataChanged: {
           
            RPZUser updated(data.toHash());
            auto updatedId = updated.id();
            auto existing = this->_sessionUsers.value(updatedId);

            {
                QMutexLocker l(&this->_m_sessionUsers);

                //remove from session users
                this->_sessionUsers.insert(updatedId, updated);

            }

            emit userDataChanged(updated);

            if(!existing.isEmpty()) {
                if(existing.whisperTargetName() != updated.whisperTargetName()) {
                    emit whisperTargetsChanged();
                }
            }

        }
        break;

        case RPZJSON::Method::AckIdentity: {
            
            {
                //store our identity
                QMutexLocker l(&this->_m_self);
                this->_self = RPZUser(data.toHash());
            }

            emit selfIdentityAcked(this->_self);

        }
        break;

        case RPZJSON::Method::Message: {
            RPZMessage msg(data.toHash());
            emit receivedMessage(msg);
        }
        break;

        case RPZJSON::Method::MapChangedHeavily:
        case RPZJSON::Method::MapChanged: {
            
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
                    qDebug() << qUtf8Printable(QStringLiteral(u"Assets : missing %1 asset(s)").arg(count));
                    this->_askForAssets(missingAssetIds);
                }

            }

            AlterationHandler::get()->queueAlteration(this, *payload);
        }   
        break;

        case RPZJSON::Method::RequestedAsset: {
            emit receivedAsset(data.toHash());
        }
        break;

        case RPZJSON::Method::ServerResponse: {
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
            msg = tr("Host closed the connection.");
            break;
        case QAbstractSocket::HostNotFoundError:
            msg = tr("Host could not be found. Please check the hostname and port number.");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            msg = tr("Connection has been refused by remote host.");
            break;
        default:
            msg = tr("An error has occured : %1").arg(this->_sock->socket()->errorString());
                                   
    }

    emit connectionStatus(msg, true);
    qDebug() << "RPZClient : :" << qUtf8Printable(msg);

    emit closed();
}


void RPZClient::sendMessage(const RPZMessage &message) {
    auto msg = RPZMessage(message);
    this->_sock->sendJSON(RPZJSON::Method::Message, msg);
}

void RPZClient::sendMapHistory(const ResetPayload &historyPayload) {
    this->_sock->sendJSON(RPZJSON::Method::MapChangedHeavily, historyPayload);
}

void RPZClient::notifyCharacterChange(const RPZCharacter &changed) {
    
    {
        QMutexLocker l(&this->_m_self);
        this->_self.setCharacter(changed);
    }

    emit selfIdentityChanged(this->_self);

    this->_sock->sendJSON(RPZJSON::Method::CharacterChanged, changed);
}

void RPZClient::_askForAssets(const QSet<RPZAssetHash> &ids) {
    QVariantList list;
    for(auto &id : ids) list.append(id);
    this->_sock->sendJSON(RPZJSON::Method::AskForAssets, list);
}

void RPZClient::changeAudioPosition(qint64 newPositionInMsecs) {
    this->_sock->sendJSON(RPZJSON::Method::AudioStreamPositionChanged, newPositionInMsecs);
}

void RPZClient::setAudioStreamPlayState(bool isPlaying) {
    this->_sock->sendJSON(RPZJSON::Method::AudioStreamPlayingStateChanged, isPlaying);
}

void RPZClient::defineAudioSourceState(const StreamPlayStateTracker &state) {
    this->_sock->sendJSON(RPZJSON::Method::AudioStreamUrlChanged, state);
}

void RPZClient::_onSending() {
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientIsSending");
}

void RPZClient::_onSent() {
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientStoppedSending");
}