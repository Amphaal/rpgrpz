#include "RPZClient.h"

RPZClient::RPZClient(const QString &socketStr, const QString &displayName, const RPZCharacter &toIncarnate) : 
    AlterationActor(Payload::Source::RPZClient),
    JSONLogger(QStringLiteral(u"[Client]")), 
    _userDisplayName(displayName),
    _characterToIncarnate(toIncarnate) { 
    
    //split socket str
    auto parts = socketStr.split(QStringLiteral(u":"), QString::SplitBehavior::SkipEmptyParts);
    this->_domain = parts.value(0, QStringLiteral(u"localhost"));
    this->_port = parts.value(1, AppContext::UPNP_DEFAULT_TARGET_PORT);

}

bool RPZClient::hasReceivedInitialMap() const {
    return this->_initialMapSetupReceived;
}

void RPZClient::_initSock() {
    this->_serverSock = new JSONSocket(this, this);

    QObject::connect(
        this->_serverSock, &JSONSocket::JSONReceived,
        this, &RPZClient::_routeIncomingJSON
    );

    QObject::connect(
        this->_serverSock->socket(), &QAbstractSocket::connected,
        this, &RPZClient::_onConnected
    );

    QObject::connect(
        this->_serverSock, &JSONSocket::sending,
        this, &RPZClient::_onSending
    );

    QObject::connect(
        this->_serverSock, &JSONSocket::sent,
        this, &RPZClient::_onSent
    );

    QObject::connect(
        this->_serverSock, &JSONSocket::ackedBatch,
        this, &RPZClient::_onBatchAcked
    );

    QObject::connect(
        this->_serverSock, &JSONSocket::batchDownloading,
        this, &RPZClient::_onBatchDownloading
    );

    QObject::connect(
        this->_serverSock->socket(), &QAbstractSocket::disconnected,
        this, &RPZClient::_onDisconnect
    );

    QObject::connect(
        this->_serverSock->socket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
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
    if(this->_serverSock) delete this->_serverSock;
}

void RPZClient::run() {

    //prerequisites
    if(this->_userDisplayName.isEmpty()) {
        emit connectionStatus(tr("Username required !"), true);
        emit closed();
        return;
    }

    this->_initSock();

    //connect...
    this->_serverSock->socket()->connectToHost(this->_domain, this->_port.toInt());

}

void RPZClient::_onDisconnect() {
    emit connectionStatus(tr("Disconnected from server"));
    this->log("disconnected from server");
}

void RPZClient::_onConnected() {
    
    RPZHandshake handshake(
        this->_userDisplayName,
        this->_characterToIncarnate
    );

    //tell the server your username
    this->_serverSock->sendToSocket(
        RPZJSON::Method::Handshake,
        handshake
    );

}

void RPZClient::_handleAlterationRequest(const AlterationPayload &payload) {

    //ignore packages from server
    if(payload.source() == this->source()) return;
    if(payload.source() == Payload::Source::RPZServer) return;

    //prevent alteration propagation to server if not host
    if(this->_myUser().role() != RPZUser::Role::Host) return;

    //ignore alteration requests when socket is not connected
    if(this->_serverSock->socket()->state() != QAbstractSocket::ConnectedState) return;

    //if not routable, instant return 
    if(!payload.isNetworkRoutable()) return;

    // this->payloadTrace(payload);

    //send json
    auto method = payload.type() == Payload::Alteration::Reset ? RPZJSON::Method::MapChangedHeavily : RPZJSON::Method::MapChanged;
    this->_serverSock->sendToSocket(method, payload);
}

const QString RPZClient::getConnectedSocketAddress() const {
    return this->_domain + ":" + this->_port;
}

const RPZUser RPZClient::identity() const {
    QMutexLocker l(&this->_m_sessionUsers);
    return this->_sessionUsers.value(this->_myUserId);
}

const RPZMap<RPZUser> RPZClient::sessionUsers() const {
    QMutexLocker l(&this->_m_sessionUsers);
    return this->_sessionUsers;
}

void RPZClient::_routeIncomingJSON(JSONSocket* target, const RPZJSON::Method &method, const QVariant &data) {
    
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientIsReceiving");

    switch(method) {

        case RPZJSON::Method::QuickDrawHappened: {
            auto qd = RPZQuickDraw(data.toHash());
            emit quickDrawReceived(qd);
        }
        break;

        case RPZJSON::Method::AvailableAssetsToUpload: {
            
            //cast
            QVector<RPZAsset::Hash> out;
            for(const auto &e : data.toList()) out += e.toString();

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
            this->_serverSock->socket()->disconnectFromHost();
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
            StreamPlayStateTracker state(data.toHash());
            emit audioSourceStateChanged(state);
        }
        break;
        
        case RPZJSON::Method::ChatLogHistory: {
            QVector<RPZMessage> msgs;
            for(const auto &rawMsg : data.toList()) {
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
                
                //inserts
                for(const auto &rUser : data.toList()) {
                    
                    RPZUser user(rUser.toHash());
                    this->_sessionUsers.insert(user.id(), user);

                    this->_registerTokenAttribution(user);

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

                this->_registerTokenAttribution(user);

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

                //from from token list
                this->_playerIdsWithoutToken.remove(idToRemove);

            }

            emit userLeftServer(out);
            emit whisperTargetsChanged();

        }
        break;

        case RPZJSON::Method::UserDataChanged: {
           
            RPZUser updated(data.toHash());
            auto updatedId = updated.id();
            auto existing = this->_sessionUsers.value(updatedId); //pickup copy before update

            {
                QMutexLocker l(&this->_m_sessionUsers);

                //replace current session user with updated one
                this->_sessionUsers.insert(updatedId, updated);

                this->_registerTokenAttribution(updated);

            }

            emit userDataChanged(updated);

            if(existing.isEmpty()) break;
            if(existing.whisperTargetName() == updated.whisperTargetName()) break;
            
            emit whisperTargetsChanged();

        }
        break;

        case RPZJSON::Method::AckIdentity: {
            
            RPZUser copy;

            {
                //store our identity
                QMutexLocker l(&this->_m_sessionUsers);
                this->_myUserId = data.toULongLong();

                auto &mUser = this->_myUser();
                Authorisations::defineHostAbility(mUser);

                copy = mUser;

            }

            emit selfIdentityAcked(copy);

        }
        break;

        case RPZJSON::Method::Message: {
            RPZMessage msg(data.toHash());
            emit receivedMessage(msg);
        }
        break;

        case RPZJSON::Method::MapChangedHeavily: {
            this->_initialMapSetupReceived = true;
        }

        case RPZJSON::Method::MapChanged: {
            
            //to sharedPointer for type casts
            auto payload = Payloads::autoCast(data.toHash());

            //if atom weilder, check if assets are missing
            if(auto mPayload = dynamic_cast<AtomsWielderPayload*>(payload.data())) { 
                
                //compare assets in map with assets in db 
                auto missingAssetHashes = mPayload->assetHashes();
                auto handledMissingAssetHashes = AssetsDatabase::get()->getStoredAssetHashes();
                missingAssetHashes.subtract(handledMissingAssetHashes);
                
                //if missing assets, request them
                if(auto count = missingAssetHashes.count()) {
                    this->log(QStringLiteral(u"Assets : missing %1 asset(s)").arg(count));
                    this->_askForAssets(missingAssetHashes);
                }

            }

            AlterationHandler::get()->queueAlteration(this, *payload);
        }   
        break;

        case RPZJSON::Method::RequestedAsset: {
            RPZAssetImportPackage package(data.toHash());
            emit receivedAsset(package);
        }
        break;

        case RPZJSON::Method::ServerResponse: {
            RPZResponse response(data.toHash());
            emit serverResponseReceived(response);
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
            msg = tr("An error has occured : %1").arg(this->_serverSock->socket()->errorString());
                                   
    }

    this->log(msg);
    
    emit connectionStatus(msg, true);
    emit closed();
}

void RPZClient::_registerTokenAttribution(const RPZUser &user) {
    
    if (user.role() != RPZUser::Role::Player) return;
    
    if(!user.playerTokenAtomId()) {
        this->_playerIdsWithoutToken.insert(user.id());
    } 
    
    else {
        this->_playerIdsWithoutToken.remove(user.id());
    }

}

void RPZClient::sendMessage(const RPZMessage &message) {
    auto msg = RPZMessage(message);
    this->_serverSock->sendToSocket(RPZJSON::Method::Message, msg);
}

void RPZClient::sendMapHistory(const ResetPayload &historyPayload) {
    this->_serverSock->sendToSocket(RPZJSON::Method::MapChangedHeavily, historyPayload);
}

RPZUser& RPZClient::_myUser() {
    return this->_sessionUsers[this->_myUserId];
}

void RPZClient::notifyCharacterChange(const RPZCharacter &changed) {
    
    {
        QMutexLocker l(&this->_m_sessionUsers);
        this->_myUser().setCharacter(changed);
    }

    auto copy = this->identity();
    emit selfIdentityChanged(copy);

    this->_serverSock->sendToSocket(RPZJSON::Method::CharacterChanged, changed);
}

void RPZClient::_askForAssets(const QSet<RPZAsset::Hash> &ids) {
    QVariantList list;
    for(const auto &id : ids) list.append(id);
    this->_serverSock->sendToSocket(RPZJSON::Method::AskForAssets, list);
}

void RPZClient::changeAudioPosition(qint64 newPositionInMsecs) {
    this->_serverSock->sendToSocket(RPZJSON::Method::AudioStreamPositionChanged, newPositionInMsecs);
}

void RPZClient::sendQuickdraw(const RPZQuickDraw &qd) {
    this->_serverSock->sendToSocket(RPZJSON::Method::QuickDrawHappened, qd);
}

void RPZClient::setAudioStreamPlayState(bool isPlaying) {
    this->_serverSock->sendToSocket(RPZJSON::Method::AudioStreamPlayingStateChanged, isPlaying);
}

void RPZClient::defineAudioSourceState(const StreamPlayStateTracker &state) {
    this->_serverSock->sendToSocket(RPZJSON::Method::AudioStreamUrlChanged, state);
}

void RPZClient::_onSending() {
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientIsSending");
}

void RPZClient::_onSent(bool success) {
    QMetaObject::invokeMethod(ProgressTracker::get(), "clientStoppedSending");
}

const QList<RPZCharacter> RPZClient::unpairedUserCharacters() const {
    
    QList<RPZCharacter> out;

    QMutexLocker l(&this->_m_sessionUsers);
    for(const auto &userId : this->_playerIdsWithoutToken) {
        auto &user = this->_sessionUsers[userId];
        out += user.character();
    }

    return out;

}