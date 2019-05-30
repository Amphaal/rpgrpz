#pragma once

#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/shared/models/entities/RPZMessage.hpp"

#include "src/helpers/_appContext.h"

class RPZClient : public JSONSocket, public JSONRouter {
    
    Q_OBJECT

    public:
        RPZClient(QObject* parent, const QString &displayname, const QString &domain, const QString &port);
        ~RPZClient();
        QString getConnectedSocketAddress();
        
        void run();
    
        //
        void sendMessage(const QString &messageToSend);
        void sendMapChanges(const QVariantHash &changes, bool isHistory);
        void askForAsset(const QString &assetId);
        
        //helper
        void informAssetSucessfulInsertion(const QString &assetId);

    signals:
        void receivedMessage(const QVariantHash &message);
        void loggedUsersUpdated(const QVariantHash &users);
        void ackIdentity(const QVariantHash &user);
        void receivedLogHistory(const QVariantList &messages);
        void error(const QString &errMessage);
        void mapChanged(const QVariantHash &data);
        void beenAskedForMapHistory();
        void receivedAsset(const QVariantHash &package);
        void assetSucessfullyInserted(const QString &assetId);

    private:       
        QString _domain;
        QString _port;
        QString _name;

        void _onConnected();
        void _error(QAbstractSocket::SocketError _socketError);
        void _onDisconnect();
        
        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;

};