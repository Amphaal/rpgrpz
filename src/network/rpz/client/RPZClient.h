#pragma once

#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/shared/models/RPZMessage.hpp"
#include "src/shared/models/RPZHandshake.hpp"

#include "src/helpers/_appContext.h"

class RPZClient : public JSONSocket, public JSONRouter {
    
    Q_OBJECT

    public:
        RPZClient(QObject* parent, const QString &displayname, const QString &domain, const QString &port);
        ~RPZClient();

        QString getConnectedSocketAddress();
        void run();
    
        //
        void sendMessage(QVariantHash &message);
        void sendMapChanges(QVariantHash &payload);
        void askForAsset(const QString &assetId);
        
        //helper
        void informAssetSucessfulInsertion(const QString &assetId);

        RPZUser identity();
        QVector<RPZUser> sessionUsers();

    signals:
        void connectionStatus(const QString &statusMessage, bool isError = false);

        void receivedMessage(const QVariantHash &message);
        void serverResponseReceived(const QVariantHash &reponse);
        void ackIdentity(const QVariantHash &user);
        
        void mapChanged(QVariantHash &payload);
        void beenAskedForMapHistory();

        void assetSucessfullyInserted(const QString &assetId);
        void receivedAsset(const QVariantHash &package);

        void loggedUsersUpdated(const QVariantList &users);
        void receivedLogHistory(const QVariantList &messages);

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

};