#pragma once

#include <QDebug>
#include <QTcpServer>
#include <QHostAddress>
#include <QDataStream>
#include <QVector>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QMutex>

#include "ServerThread.h"

#include "src/helpers/_const.cpp"

class ChatServer : public ServerThread { 
    
    Q_OBJECT

    public:
        ChatServer();
        ~ChatServer();
        void stop();
        void run() override;
        //void start();

    private:
        QVector<QTcpSocket*> _clientSockets;
        QTcpServer* _server;
        
        QMutex _mutex;
        bool _stopped = false;
        bool _isStopped();

        QVector<QString> _messages;
        void _sendWelcomeMessage(QTcpSocket *clientSocket);
        void _handleIncomingMessages(QTcpSocket * clientSocket);

        void _onNewConnection();
};