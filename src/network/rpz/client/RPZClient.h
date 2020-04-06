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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

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
#include "src/shared/models/RPZQuickDraw.hpp"

#include "src/helpers/_appContext.h"
#include "src/shared/async-ui/AlterationInteractor.hpp"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/shared/audio/StreamPlayStateTracker.hpp"

#include "src/network/rpz/_any/JSONLogger.hpp"
#include "src/helpers/Authorisations.hpp"
#include "src/shared/models/network/RPZGameSession.hpp"
#include "src/shared/async-ui/AlterationHandler.h"

class RPZClient : public QObject, public AlterationInteractor, public JSONLogger {

    Q_OBJECT

    public:
        RPZClient(const QString &socketStr, const QString &displayName, const RPZCharacter &toIncarnate);
        ~RPZClient();
        
        const QString getConnectedSocketAddress() const; //safe
        bool hasReceivedInitialMap() const; //safe

        const RPZUser identity() const; //safe
        const RPZMap<RPZUser> sessionUsers() const; //safe
        const QList<RPZCharacter::UserBound> sessionCharacters() const; //safe
        const RPZCharacter sessionCharacter(const RPZCharacter::Id &characterId) const; //safe

    public slots:
        void run();

        void sendMessage(const RPZMessage &message);
        void defineAudioSourceState(const StreamPlayStateTracker &state);
        void changeAudioPosition(qint64 newPositionInMsecs);
        void setAudioStreamPlayState(bool isPlaying);
        void sendMapHistory(const ResetPayload &historyPayload);
        void notifyCharacterChange(const RPZCharacter &changed);
        void sendQuickdraw(const RPZQuickDraw &qd);

    signals:
        void connectionStatus(const QString &statusMessage, bool isError = false);
        void closed();

        void receivedMessage(const RPZMessage &message);
        void serverResponseReceived(const RPZResponse &reponse);
        
        void quickDrawReceived(const RPZQuickDraw &qd);

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

    private:
        enum class CharacterRegistration {
            In,
            Out
        };
        JSONSocket* _serverSock = nullptr;   
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
        void _error(QAbstractSocket::SocketError _socketError);
        void _onDisconnect();
        void _onSending();
        void _onSent(bool success);
        void _onBatchAcked(RPZJSON::Method method, qint64 batchSize);
        void _onBatchDownloading(RPZJSON::Method method, qint64 downloaded);
        void _askForAssets(const QSet<RPZAsset::Hash> &ids);

        void _routeIncomingJSON(JSONSocket* target, const RPZJSON::Method &method, const QVariant &data);

    private slots:
        void _handleAlterationRequest(const AlterationPayload &payload);
};