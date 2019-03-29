#pragma once

#include <QtCore/QThread>

#include <QTcpSocket>
#include <QDataStream>
#include <QMessageBox>

class ChatClient : public QObject {
    
    Q_OBJECT

    public:
        ChatClient(QString displayname, QString domain, QString port);
        void close();
        QString getConnectedSocketAddress();
        void sendMessage(QString messageToSend);
    
    signals:
        void connected();
        void error(const std::string errMessage);

    private:
        QDataStream _in;
        QTcpSocket* _socket = nullptr;
        
        QString _domain;
        QString _port;
        QString _dn;

        void _onRR();
        void _error(QAbstractSocket::SocketError _socketError);

        void _tryConnection();
};