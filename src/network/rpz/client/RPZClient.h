#pragma once

#include <QThread>

#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>

#include <QMutex>


#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/shared/models/RPZMessage.h"
#include "src/shared/models/RPZResponse.h"
#include "src/shared/models/RPZHandshake.h"

#include "src/helpers/_appContext.h"
#include "src/shared/async-ui/AlterationActor.hpp"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/shared/audio/StreamPlayStateTracker.hpp"

class RPZClient : public QObject, public AlterationActor, public JSONRouter {

    Q_OBJECT

    public:
        RPZClient(const QString &displayname, const QString &domain, const QString &port);
        ~RPZClient();
        
        QString getConnectedSocketAddress() const; //safe

        RPZUser identity() const; //safe
        QVector<RPZUser> sessionUsers() const; //safe

    public slots:
        void run();

        void sendMessage(const RPZMessage &message);
        void defineAudioSourceState(const StreamPlayStateTracker &state);
        void changeAudioPosition(int newPosition);
        void setAudioStreamPlayState(bool isPlaying);
        void sendMapHistory(const ResetPayload &historyPayload);

    signals:
        void connectionStatus(const QString &statusMessage, bool isError = false);
        void closed();

        void receivedMessage(const RPZMessage &message);
        void serverResponseReceived(const RPZResponse &reponse);
        void ackIdentity(const RPZUser &user);
        
        void mapChanged(const AlterationPayload &payload);
        void beenAskedForMapHistory();

        void availableAssetsFromServer(const QVector<RPZAssetHash> &availableIds);
        void receivedAsset(const RPZAssetImportPackage &package);

        void loggedUsersUpdated(const QVector<RPZUser> &users);
        void receivedLogHistory(const QVector<RPZMessage> &messages);

        void audioSourceStateChanged(const StreamPlayStateTracker &state);
        void audioPositionChanged(int newPos);
        void audioPlayStateChanged(bool isPlaying);

    private:   
        JSONSocket* _sock = nullptr;   
        
        QString _domain;
        QString _port;
        QString _name;

        RPZUser _self;
        mutable QMutex _m_self;

        QVector<RPZUser> _sessionUsers;
        mutable QMutex _m_sessionUsers;

        void _initSock();

        void _onConnected();
        void _error(QAbstractSocket::SocketError _socketError);
        void _onDisconnect();
        void _onSending();
        void _onSent();
        void _onBatchAcked(JSONMethod method, qint64 batchSize);
        void _onBatchDownloading(JSONMethod method, qint64 downloaded);

        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;

    private slots:
        void _handleAlterationRequest(const AlterationPayload &payload);
        void _askForAssets(const QSet<RPZAssetHash> &ids);

};