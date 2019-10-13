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

        case JSONMethod::UserIn: {
            
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

        case JSONMethod::UserOut: {
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

        case JSONMethod::UserDataChanged: {
           
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

        case JSONMethod::AckIdentity: {
            
            {
                //store our identity
                QMutexLocker l(&this->_m_self);
                this->_self = RPZUser(data.toHash());
            }

            emit selfIdentityAcked(this->_self);

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
                    qDebug() << qUtf8Printable(QString("Assets : missing %1 asset(s)").arg(count));
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
    this->_sock->sendJSON(JSONMethod::Message, msg);
}

void RPZClient::sendMapHistory(const ResetPayload &historyPayload) {
    this->_sock->sendJSON(JSONMethod::MapChangedHeavily, historyPayload);
}

void RPZClient::notifyCharacterChange(const RPZCharacter &changed) {
    
    {
        QMutexLocker l(&this->_m_self);
        this->_self.setCharacter(changed);
    }

    emit selfIdentityChanged(this->_self);

    this->_sock->sendJSON(JSONMethod::CharacterChanged, changed);
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