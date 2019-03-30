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

#include "src/helpers/_const.cpp"

class ChatServer : public ServerThread { 
    
    Q_OBJECT

    public:
        ChatServer();
        void run() override;

    private:
        QMap<QTcpSocket*, QTcpSocket*> _clientSockets;
        QTcpServer* _server;

        QStringList _messages;
        void _sendStoredMessages(QTcpSocket * clientSocket);
        void _handleIncomingMessages(QTcpSocket * clientSocket);
        void _broadcastMessage(QString messageToBroadcast);

        void _sendJSONtoSocket(QTcpSocket * clientSocket, QJsonDocument doc);
        void _onNewConnection();
};