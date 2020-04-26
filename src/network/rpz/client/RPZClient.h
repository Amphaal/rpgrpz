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

#pragma once

#include <QThread>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>
#include <QMutex>

#include "src/network/rpz/_any/JSONSocket.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/shared/models/messaging/RPZMessage.h"
#include "src/shared/models/messaging/RPZResponse.h"
#include "src/shared/models/network/RPZHandshake.h"
#include "src/shared/models/RPZQuickDrawBits.hpp"

#include "src/helpers/_appContext.h"
#include "src/shared/async-ui/AlterationInteractor.hpp"
#include "src/shared/audio/StreamPlayStateTracker.hpp"

#include "src/network/rpz/_any/JSONLogger.hpp"
#include "src/helpers/Authorisations.hpp"
#include "src/shared/models/network/RPZGameSession.hpp"
#include "src/shared/async-ui/AlterationHandler.h"
#include "src/shared/models/RPZSharedDocument.hpp"
#include "src/shared/models/RPZPing.hpp"

class RPZClient :  public JSONSocket, public AlterationInteractor, public JSONLogger {
    Q_OBJECT

 public:
    RPZClient(const QString &socketStr, const QString &displayName, const RPZCharacter &toIncarnate);

    const QString getConnectedSocketAddress() const;  // safe
    bool hasReceivedInitialMap() const;  // safe

    const RPZUser identity() const;  // safe
    const RPZMap<RPZUser> sessionUsers() const;  // safe
    const QList<RPZCharacter::UserBound> sessionCharacters() const;  // safe
    const RPZCharacter sessionCharacter(const RPZCharacter::Id &characterId) const;  // safe

    void run();

 public slots:
    void sendMessage(const RPZMessage &message);
    void defineAudioSourceState(const StreamPlayStateTracker &state);
    void changeAudioPosition(qint64 newPositionInMsecs);
    void setAudioStreamPlayState(bool isPlaying);
    void sendMapHistory(const ResetPayload &historyPayload);
    void notifyCharacterChange(const RPZCharacter &changed);
    void sendQuickdraw(const RPZQuickDrawBits &qd);
    void addSharedDocument(const RPZSharedDocument::FileHash &hash, const RPZSharedDocument::DocumentName &documentName);
    void requestSharedDocument(const RPZSharedDocument::FileHash &hash);
    void notifyPing(const QPointF &pingPosition);

 signals:
    void connectionStatus(const QString &statusMessage, bool isError = false);
    void ended();

    void receivedMessage(const RPZMessage &message);
    void serverResponseReceived(const RPZResponse &reponse);

    void quickDrawBitsReceived(const RPZQuickDrawBits &qd);

    void availableAssetsFromServer(const QVector<RPZAsset::Hash> &availableIds);
    void receivedAsset(const RPZAssetImportPackage &package);

    void userLeftServer(const RPZUser &userOut);
    void userJoinedServer(const RPZUser &newUser);
    void userDataChanged(const RPZUser &updatedUser);
    void whisperTargetsChanged();

    void gameSessionReceived(const RPZGameSession &gameSession);
    void characterImpersonated(const RPZCharacter::Id &impersonatedCharacterId);

    void charactersCountChanged();

    void audioSourceStateChanged(const StreamPlayStateTracker &state);
    void audioPositionChanged(qint64 newPosInMsecs);
    void audioPlayStateChanged(bool isPlaying);

    void sharedDocumentAvailable(const RPZSharedDocument::FileHash &documentHash, const QString &documentName);
    void sharedDocumentReceived(const RPZSharedDocument &sharedDocument);

    void pingHappened(const RPZPing &ping);

 private:
    enum class CharacterRegistration {
        In,
        Out
    };
    bool _initialMapSetupReceived = false;

    QString _domain;
    QString _port;
    QString _userDisplayName;
    RPZCharacter _handshakeCharacter;

    RPZUser::Id _myUserId;
    RPZUser& _myUser();

    void _onMapChangeReceived(AlterationPayload *castedPayload, bool isHeavyChange);
    void _registerSessionUsers(const RPZGameSession &gameSession);

    RPZMap<RPZUser> _sessionUsers;
    mutable QMutex _m_sessionUsers;

    bool _hasPendingCharactersRegistration = false;
    void _mayRegisterAsCharacterized(const RPZUser &user, const CharacterRegistration &type);
    void _checkPendingCharactersRegistration();
    QSet<RPZUser::Id> _characterizedUserIds;
    QHash<RPZCharacter::Id, RPZUser::Id> _associatedUserIdByCharacterId;

    void _initSock();

    void _onConnected();
    void _onDisconnect();
    void _onError(QAbstractSocket::SocketError _socketError);

    void _askForAssets(const QSet<RPZAsset::Hash> &ids);

    void _onPayloadReceived(const RPZJSON::Method &method, const QVariant &data);
    void _handleAlterationRequest(const AlterationPayload &payload);
};
