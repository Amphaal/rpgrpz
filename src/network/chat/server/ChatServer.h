#pragma once

#include <QtCore/QThread>
#include <QDebug>
#include <QTcpServer>
#include <QHostAddress>
#include <QDataStream>
#include <QVector>
#include <QAbstractSocket>
#include <QTcpSocket>

#include "src/helpers/_const.cpp"
#include "ChatServerThread.h"

class ChatServer : public ChatServerThread { 
    
    public:
        ChatServer(QObject * parent = nullptr);
        void run() override;

    private:
        QTcpServer* _server;
        QVector<QTcpSocket*> _clients;

        void _onNewConnection();
        void _onFinished();
};