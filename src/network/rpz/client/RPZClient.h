#pragma once

#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/RPZMessage.hpp"

#include "src/helpers/_const.hpp"

class RPZClient : public JSONSocket, public JSONRouter {
    
    Q_OBJECT

    public:
        RPZClient(QObject* parent, const QString &displayname, const QString &domain, const QString &port);
        ~RPZClient();
        QString getConnectedSocketAddress();
        
        void run();
    
    public slots:
        void sendMessage(const QString &messageToSend);
        void sendMapHistory(const QVariantList &history);

    signals:
        void receivedMessage(const QVariantHash &message);
        void loggedUsersUpdated(const QVariantHash &users);
        void receivedLogHistory(const QVariantList &messages);
        void error(const std::string &errMessage);
        void hostMapChanged(const QVariantList &data);
        void beenAskedForMapHistory();

    private:
        
        QString _domain;
        QString _port;
        QString _name;

        void _onConnected();
        void _error(QAbstractSocket::SocketError _socketError);
        void _onDisconnect();
        
        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;

};