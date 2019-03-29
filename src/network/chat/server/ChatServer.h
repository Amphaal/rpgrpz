#pragma once

#include <QtCore/QThread>
#include <QDebug>
#include <QTcpServer>
#include <QHostAddress>
#include <QDataStream>
#include <QVector>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QMutex>

#include "src/helpers/_const.cpp"

class ChatServer : public QObject { 
    
    Q_OBJECT

    public:
        ChatServer();
        ~ChatServer();
        void stop();
        void start();

    signals:
        void newConnectionReceived(std::string ip);

    private:
        QTcpServer* _server;
        
        QMutex _mutex;
        bool _stopped = false;
        bool _isStopped();

        QVector<QString> _messages;
        void _sendWelcomeMessage(QTcpSocket *clientSocket);
        void _handleIncomingMessages(QTcpSocket * clientSocket);
};