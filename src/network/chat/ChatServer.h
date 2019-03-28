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

class ChatServer : public QThread { 
    
    Q_OBJECT 
    
    public:
        ChatServer();
        void run() override;

    private:
        QTcpServer* _server;
        QVector<QTcpSocket*> _clients;

        void _onNewConnection();
        void _onFinished();
};