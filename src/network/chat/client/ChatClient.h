#pragma once

#include <QtCore/QThread>

#include <QTcpSocket>
#include <QDataStream>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "src/helpers/_const.cpp"

class ChatClient : public QObject {
    
    Q_OBJECT

    public:
        ChatClient(QString displayname, QString domain, QString port);
        void close();
        QString getConnectedSocketAddress();
        void sendMessage(QString messageToSend);
        void tryConnection();

    signals:
        void connected();
        void receivedMessage(const std::string message);
        void historyReceived();
        void error(const std::string errMessage);

    private:
        QDataStream _in;
        QTcpSocket* _socket = nullptr;
        
        QString _domain;
        QString _port;
        QString _name;

        void _onRR();
        void _error(QAbstractSocket::SocketError _socketError);
        void _JSONTriage(QByteArray &potentialJSON);

};