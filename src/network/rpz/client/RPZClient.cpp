// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "RPZClient.h"

RPZClient::RPZClient(const QString &socketStr, const QString &displayName, const RPZCharacter &toIncarnate) :
    JSONSocket(nullptr, this),
    AlterationInteractor(Payload::Interactor::RPZClient),
    JSONLogger(QStringLiteral(u"[Client]")),
    _userDisplayName(displayName),
    _handshakeCharacter(toIncarnate) {
    // split socket str
    auto parts = socketStr.split(QStringLiteral(u":"), QString::SplitBehavior::SkipEmptyParts);
    this->_domain = parts.value(0, QStringLiteral(u"localhost"));
    this->_port = parts.value(1, AppContext::UPNP_DEFAULT_TARGET_PORT);
}

RPZClient::~RPZClient() {
    this->log("disconnected from server");
}

bool RPZClient::hasReceivedInitialMap() const {
    return this->_initialMapSetupReceived;
}

void RPZClient::quit() {
    emit ended(QString());
}

void RPZClient::_onError(QAbstractSocket::SocketError _socketError) {
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
            msg = tr("An error has occured : %1").arg(this->errorString());
            break;
    }

    this->log(msg);
    emit ended(msg);
}

void RPZClient::_initSock() {
    QObject::connect(
        this, &JSONSocket::PayloadReceived,
        this, &RPZClient::_onPayloadReceived
    );

    QObject::connect(
        this, &QAbstractSocket::connected,
        this, &RPZClient::_onConnected
    );

    QObject::connect(
        this, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        this, &RPZClient::_onError
    );

    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &RPZClient::_handleAlterationRequest
    );
}

void RPZClient::run() {
    // prerequisites
    if (this->_userDisplayName.isEmpty()) {
        emit ended(tr("Username required !"));
        return;
    }

    this->_initSock();

    // connect...
    this->connectToHost(this->_domain, this->_port.toInt());
}

void RPZClient::_onConnected() {
    RPZHandshake handshake(
        this->_userDisplayName,
        this->_handshakeCharacter
    );

    // tell the server your username
    this->sendToSocket(
        RPZJSON::Method::Handshake,
        handshake
    );
}

void RPZClient::_handleAlterationRequest(const AlterationPayload &payload) {
    // ignore packages from server
    auto pSource = payload.source();
    if (pSource == this->interactorId()) return;  // prevent rehandling
    if (pSource == Payload::Interactor::RPZServer) return;

    // prevent alteration propagation to server if not host
    if (this->_myUser().role() == RPZUser::Role::Observer) return;

    // ignore alteration requests when socket is not connected
    if (this->state() != QAbstractSocket::ConnectedState) return;

    // if not routable, instant return
    if (!payload.isNetworkRoutable()) return;

    this->payloadTrace(payload);

    // send json
    auto method = payload.type() == Payload::Alteration::Reset ? RPZJSON::Method::MapChangedHeavily : RPZJSON::Method::MapChanged;
    this->sendToSocket(method, payload);
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


void RPZClient::_onMapChangeReceived(AlterationPayload *castedPayload, bool isHeavyChange) {
    if (isHeavyChange) this->_initialMapSetupReceived = true;

    // if atom weilder, check if assets are missing
    if (auto mPayload = dynamic_cast<const AtomsWielderPayload*>(castedPayload)) {
        // compare assets in map with assets in db
        auto missingAssetHashes = mPayload->assetHashes();
        auto handledMissingAssetHashes = AssetsDatabase::get()->getStoredAssetHashes();
        missingAssetHashes.subtract(handledMissingAssetHashes);

        // if missing assets, request them
        if (auto count = missingAssetHashes.count()) {
            this->log(QStringLiteral(u"Assets : missing %1 asset(s)").arg(count));
            this->_askForAssets(missingAssetHashes);
        }
    }

    // propagate
    AlterationHandler::get()->queueAlteration(this, *castedPayload);
}

void RPZClient::_checkPendingCharactersRegistration() {
    if (!this->_hasPendingCharactersRegistration) return;
    emit charactersCountChanged();
    this->_hasPendingCharactersRegistration = false;
}


void RPZClient::_mayRegisterAsCharacterized(const RPZUser &user, const CharacterRegistration &type) {
    if (user.role() != RPZUser::Role::Player) return;

    auto userId = user.id();
    if (!userId) return;

    auto characterId = user.character().id();

    switch (type) {
        case CharacterRegistration::In: {
            this->_characterizedUserIds.insert(userId);
            this->_associatedUserIdByCharacterId.insert(characterId, userId);
        }
        break;

        case CharacterRegistration::Out: {
            this->_characterizedUserIds.remove(userId);
            this->_associatedUserIdByCharacterId.remove(characterId);
        }
        break;
    }

    this->_hasPendingCharactersRegistration = true;
}

void RPZClient::_registerSessionUsers(const RPZGameSession &gameSession) {
    // handle user
    {
        QMutexLocker l(&this->_m_sessionUsers);

        // replace all stored users
        this->_sessionUsers.clear();

        // inserts
        auto users = gameSession.users();
        for (const auto &user : users) {
            this->_sessionUsers.insert(user.id(), user);
            this->_mayRegisterAsCharacterized(user, CharacterRegistration::In);
        }

        // define self
        this->_myUserId = gameSession.selfUserId();
        auto &mUser = this->_myUser();
        Authorisations::defineHostAbility(mUser);
    }

    this->_checkPendingCharactersRegistration();
    emit whisperTargetsChanged();
}

void RPZClient::_onPayloadReceived(const RPZJSON::Method &method, const QVariant &data) {
    auto target = dynamic_cast<JSONSocket*>(this->sender());

    switch (method) {
        case RPZJSON::Method::PingHappened: {
            RPZPing ping(data.toHash());
            emit pingHappened(ping);
        }
        break;

        case RPZJSON::Method::SharedDocumentAvailable: {
            auto pair = data.toList();
            emit sharedDocumentAvailable(
                pair.value(0).toString(),
                pair.value(1).toString()
            );
        }
        break;

        case RPZJSON::Method::SharedDocumentRequested: {
            RPZSharedDocument sd(data.toHash());
            emit sharedDocumentReceived(sd);
        }
        break;

        case RPZJSON::Method::QuickDrawHappened: {
            RPZQuickDrawBits qd(data.toHash());
            emit quickDrawBitsReceived(qd);
        }
        break;

        case RPZJSON::Method::AvailableAssetsToUpload: {
            // cast
            QVector<RPZAsset::Hash> out;
            for (const auto &e : data.toList()) out += e.toString();

            // emit
            emit availableAssetsFromServer(out);
        }
        break;

        case RPZJSON::Method::ServerStatus: {
            emit ended(data.toString());
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

        case RPZJSON::Method::GameSessionSync: {
            // get game session
            RPZGameSession gs(data.toHash());

            // handle session users
            this->_registerSessionUsers(gs);

            // game session emission
            emit gameSessionReceived(gs);

            // if full session, additionnal actions
            if (gs.isFullSession()) {
                // stream state
                emit audioSourceStateChanged(gs.streamState());

                // define impersonation
                auto self = this->identity();
                emit characterImpersonated(self.character().id());

                // map change
                auto payload = gs.mapPayload();
                this->_onMapChangeReceived(&payload, true);
            }
        }
        break;

        case RPZJSON::Method::UserIn: {
            RPZUser user(data.toHash());

            {
                QMutexLocker l(&this->_m_sessionUsers);

                // add user to session users
                this->_sessionUsers.insert(user.id(), user);

                this->_mayRegisterAsCharacterized(user, CharacterRegistration::In);
            }

            this->_checkPendingCharactersRegistration();
            emit userJoinedServer(user);
            emit whisperTargetsChanged();
        }
        break;

        case RPZJSON::Method::UserOut: {
            auto idToRemove = data.toULongLong();
            RPZUser out;

            {
                QMutexLocker l(&this->_m_sessionUsers);

                // remove from session users
                out = this->_sessionUsers.take(idToRemove);

                // from from token list
                this->_mayRegisterAsCharacterized(out, CharacterRegistration::Out);
            }

            this->_checkPendingCharactersRegistration();
            emit userLeftServer(out);
            emit whisperTargetsChanged();
        }
        break;

        case RPZJSON::Method::UserDataChanged: {
            RPZUser updated(data.toHash());
            auto updatedId = updated.id();
            auto existing = this->_sessionUsers.value(updatedId);  // pickup copy before update

            {
                QMutexLocker l(&this->_m_sessionUsers);

                // replace current session user with updated one
                this->_sessionUsers.insert(updatedId, updated);
            }

            emit userDataChanged(updated);

            if (existing.isEmpty()) break;
            if (existing.whisperTargetName() == updated.whisperTargetName()) break;

            emit whisperTargetsChanged();
        }
        break;

        case RPZJSON::Method::Message: {
            RPZMessage msg(data.toHash());
            emit receivedMessage(msg);
        }
        break;

        case RPZJSON::Method::MapChangedHeavily:
        case RPZJSON::Method::MapChanged: {
            auto payload = Payloads::autoCast(data.toHash());  // to sharedPointer for type casts
            auto isHeavyChange = method == RPZJSON::Method::MapChangedHeavily;

            this->_onMapChangeReceived(payload.data(), isHeavyChange);
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
}

void RPZClient::sendMessage(const RPZMessage &message) {
    auto msg = RPZMessage(message);
    this->sendToSocket(RPZJSON::Method::Message, msg);
}

void RPZClient::sendMapHistory(const ResetPayload &historyPayload) {
    this->sendToSocket(
        RPZJSON::Method::MapChangedHeavily,
        historyPayload
    );
}

void RPZClient::addSharedDocument(const RPZSharedDocument::FileHash &hash, const RPZSharedDocument::DocumentName &documentName) {
    // out
    QVariantList out;
    out.append(hash);
    out.append(documentName);

    // send
    this->sendToSocket(
        RPZJSON::Method::SharedDocumentAvailable,
        out
    );
}

void RPZClient::requestSharedDocument(const RPZSharedDocument::FileHash &hash) {
    this->sendToSocket(
        RPZJSON::Method::SharedDocumentRequested,
        hash
    );
}

void RPZClient::notifyPing(const QPointF &pingPosition) {
    RPZPing ping(pingPosition, this->_myUserId);

    this->sendToSocket(
        RPZJSON::Method::PingHappened,
        ping
    );
}

RPZUser& RPZClient::_myUser() {
    return this->_sessionUsers[this->_myUserId];
}

void RPZClient::notifyCharacterChange(const RPZCharacter &changed) {
    {
        QMutexLocker l(&this->_m_sessionUsers);
        this->_myUser().setCharacter(changed);
    }

    emit userDataChanged(this->identity());

    this->sendToSocket(RPZJSON::Method::CharacterChanged, changed);
}

void RPZClient::_askForAssets(const QSet<RPZAsset::Hash> &ids) {
    QVariantList list;
    for (const auto &id : ids) list.append(id);
    this->sendToSocket(RPZJSON::Method::AskForAssets, list);
}

void RPZClient::changeAudioPosition(qint64 newPositionInMsecs) {
    this->sendToSocket(RPZJSON::Method::AudioStreamPositionChanged, newPositionInMsecs);
}

void RPZClient::sendQuickdraw(const RPZQuickDrawBits &qd) {
    this->sendToSocket(RPZJSON::Method::QuickDrawHappened, qd);
}

void RPZClient::setAudioStreamPlayState(bool isPlaying) {
    this->sendToSocket(RPZJSON::Method::AudioStreamPlayingStateChanged, isPlaying);
}

void RPZClient::defineAudioSourceState(const StreamPlayStateTracker &state) {
    this->sendToSocket(RPZJSON::Method::AudioStreamUrlChanged, state);
}

const QList<RPZCharacter::UserBound> RPZClient::sessionCharacters() const {
    QList<RPZCharacter::UserBound> out;

    QMutexLocker l(&this->_m_sessionUsers);
    for (const auto &userId : this->_characterizedUserIds) {
        auto user = this->_sessionUsers.value(userId);
        if (user.isEmpty()) continue;

        out += { user.color(), user.character() };
    }

    return out;
}

const RPZCharacter RPZClient::sessionCharacter(const RPZCharacter::Id &characterId) const {
    QMutexLocker l(&this->_m_sessionUsers);

    auto userId = this->_associatedUserIdByCharacterId.value(characterId);
    return this->_sessionUsers.value(userId).character();
}
