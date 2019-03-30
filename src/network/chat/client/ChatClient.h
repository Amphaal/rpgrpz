#pragma once

#include <QtCore/QThread>

#include <QTcpSocket>
#include <QDataStream>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "ClientThread.h"
#include "src/network/chat/_any/JSONSocket.h"
#include "src/network/chat/_any/JSONRouter.h"

#include "src/helpers/_const.cpp"

class ChatClient : public ClientThread, public JSONRouter {
    
    Q_OBJECT

    public:
        ChatClient(QString displayname, QString domain, QString port);
        QString getConnectedSocketAddress();
        void sendMessage(QString messageToSend);
        void run() override;

    private:
        JSONSocket* _sockWrapper = nullptr;
        
        QString _domain;
        QString _port;
        QString _name;

        void _error(QAbstractSocket::SocketError _socketError);
        void _constructorInThread();

        void _routeIncomingJSON(JSONSocket * wrapper, QString method, QVariant data) override;

};