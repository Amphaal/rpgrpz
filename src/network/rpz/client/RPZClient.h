#pragma once

#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/shared/models/RPZMessage.h"
#include "src/shared/models/RPZHandshake.h"

#include "src/helpers/_appContext.h"
#include "src/shared/atoms/AtomAlterationAcknoledger.h"

class RPZClient : public JSONSocket, public JSONRouter, public AtomAlterationAcknoledger {
    
    Q_OBJECT

    public:
        RPZClient(QObject* parent, const QString &displayname, const QString &domain, const QString &port);
        ~RPZClient();

        QString getConnectedSocketAddress();
        void run();
    
        //slots
        void sendMessage(QVariantHash &message);
        void askForAssets(const QList<RPZAssetHash> ids);
        void defineAudioStreamSource(const QString &audioStreamUrl, const QString &sourceTitle);
        void changeAudioPosition(int newPosition);
        void setAudioStreamPlayState(bool isPlaying);
        void sendMapHistory(const QVariantHash &history);

        RPZUser identity();
        QVector<RPZUser> sessionUsers();

    signals:
        void connectionStatus(const QString &statusMessage, bool isError = false);

        void receivedMessage(const QVariantHash &message);
        void serverResponseReceived(const QVariantHash &reponse);
        void ackIdentity(const QVariantHash &user);
        
        void mapChanged(const QVariantHash &payload);
        void beenAskedForMapHistory();

        void assetSucessfullyInserted(const RPZAssetMetadata &metadata);
        void receivedAsset(const QVariantHash &package);

        void loggedUsersUpdated(const QVariantList &users);
        void receivedLogHistory(const QVariantList &messages);

        void audioSourceChanged(const QString &audioSourceUrl, const QString &sourceTitle);
        void audioPositionChanged(int newPos);
        void audioPlayStateChanged(bool isPlaying);

    private:       
        QString _domain;
        QString _port;
        QString _name;

        RPZUser _self;
        QVector<RPZUser> _sessionUsers;

        void _onConnected();
        void _error(QAbstractSocket::SocketError _socketError);
        void _onDisconnect();
        
        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;
 
        QFuture<void> _handleAlterationRequest(AlterationPayload &payload, bool autoPropagate = true) override;

};