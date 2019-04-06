#pragma once

#include <QtCore/QThread>

#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "RPZCThread.h"
#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/helpers/_const.hpp"

class RPZClient : public RPZCThread, public JSONRouter {
    
    Q_OBJECT

    public:
        RPZClient(const QString &displayname, const QString &domain, const QString &port);
        QString getConnectedSocketAddress();
        void sendMessage(const QString &messageToSend);
        void run() override;

    private:
        JSONSocket* _sockWrapper = nullptr;
        
        QString _domain;
        QString _port;
        QString _name;

        void _error(QAbstractSocket::SocketError _socketError);
        void _constructorInThread();

        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;

};