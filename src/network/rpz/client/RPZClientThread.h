#pragma once

#include <QThread>

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
#include "src/shared/async-ui/AlterationAcknoledger.h"

class RPZClientThread : public QThread, public JSONRouter, public AlterationAcknoledger {

    Q_OBJECT

    public:
        RPZClientThread(QObject* parent, const QString &displayname, const QString &domain, const QString &port);
        
        QString getConnectedSocketAddress();
        void run() override;
    
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
        JSONSocket* _cli = nullptr;

        void _onConnected();
        void _error(QAbstractSocket::SocketError _socketError);
        void _onDisconnect();
        
        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;
 
        void _handleAlterationRequest(AlterationPayload &payload);

};