#pragma once

#include <QDebug>
#include <QTcpServer>
#include <QHostAddress>
#include <QDataStream>
#include <QVector>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QMap>

#include "ServerThread.h"
#include "src/network/chat/_any/JSONSocket.h"
#include "src/network/chat/_any/JSONRouter.h"

#include "src/helpers/_const.cpp"

class ChatServer : public ServerThread, public JSONRouter { 
    
    Q_OBJECT

    public:
        ChatServer();
        void run() override;

    private:
        QMap<JSONSocket*, JSONSocket*> _clientSockets;
        QMap<JSONSocket*, QString> _clientDisplayNames;
        QTcpServer* _server;

        QStringList _messages;
        void _sendStoredMessages(JSONSocket * clientSocket);
        void _broadcastMessage(QString messageToBroadcast);
        void _broadcastUsers();

        void _onNewConnection();
        QString _getSocketDisplayName(JSONSocket * clientSocket);

        void _routeIncomingJSON(JSONSocket * wrapper, QString method, QVariant data) override;
};