#pragma once

#include <QtCore/QThread>

#include <QTcpSocket>
#include <QDataStream>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "ClientThread.h"

#include "src/helpers/_const.cpp"

class ChatClient : public ClientThread {
    
    Q_OBJECT

    public:
        ChatClient(QString displayname, QString domain, QString port);
        QString getConnectedSocketAddress();
        void sendMessage(QString messageToSend);
        void run() override;

    private:
        QDataStream* _in = nullptr;
        QTcpSocket* _socket = nullptr;
        
        QString _domain;
        QString _port;
        QString _name;

        void _onRR();
        void _error(QAbstractSocket::SocketError _socketError);
        void _JSONTriage(QByteArray &potentialJSON);
        void _constructorInThread();

};