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
#include "src/shared/models/messaging/RPZHandshake.h"
#include "src/shared/models/RPZQuickDraw.hpp"

#include "src/helpers/_appContext.h"
#include "src/shared/async-ui/AlterationActor.hpp"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/shared/audio/StreamPlayStateTracker.hpp"

#include "src/network/rpz/_any/JSONLogger.hpp"

class RPZClient : public QObject, public AlterationActor, public JSONLogger {

    Q_OBJECT

    public:
        RPZClient(const QString &socketStr, const QString &displayName, const RPZCharacter &toIncarnate);
        ~RPZClient();
        
        const QString getConnectedSocketAddress() const; //safe
        bool hasReceivedInitialMap() const; //safe

        const RPZUser identity() const; //safe
        const RPZMap<RPZUser> sessionUsers() const; //safe
        const QList<RPZCharacter> unpairedUserCharacters() const; //safe

        static void resetHostAbility();
        static bool isHostAble();

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
        void selfIdentityAcked(const RPZUser &identity);
        void selfIdentityChanged(const RPZUser &updated);
        
        void quickDrawReceived(const RPZQuickDraw &qd);

        void availableAssetsFromServer(const QVector<RPZAsset::Hash> &availableIds);
        void receivedAsset(const RPZAssetImportPackage &package);

        void allUsersReceived();
        void userLeftServer(const RPZUser &userOut);
        void userJoinedServer(const RPZUser &newUser);
        void userDataChanged(const RPZUser &updatedUser);
        void whisperTargetsChanged();

        void receivedLogHistory(const QVector<RPZMessage> &messages);

        void audioSourceStateChanged(const StreamPlayStateTracker &state);
        void audioPositionChanged(qint64 newPosInMsecs);
        void audioPlayStateChanged(bool isPlaying);

    private:
        static inline bool _isHostAble = true;
        static void _defineHostAbility(const RPZUser &user);

        JSONSocket* _serverSock = nullptr;   
        bool _initialMapSetupReceived = false;

        QString _domain;
        QString _port;
        QString _userDisplayName;
        RPZCharacter _characterToIncarnate;

        RPZUser::Id _myUserId;
        RPZUser& _myUser();

        RPZMap<RPZUser> _sessionUsers;
        QSet<RPZUser::Id> _playerIdsWithoutToken;
        mutable QMutex _m_sessionUsers;

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

        //helper
        void _registerTokenAttribution(const RPZUser &user); 

    private slots:
        void _handleAlterationRequest(const AlterationPayload &payload);
};