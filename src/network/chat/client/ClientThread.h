#pragma once

#include <QThread>

class ClientThread : public QThread {
    
    Q_OBJECT

    signals:
        void connected(QString _socketAddress);
        void receivedMessage(const std::string message);
        void loggedUsersUpdated(QVariantList users);
        void historyReceived();
        void error(const std::string errMessage);
};