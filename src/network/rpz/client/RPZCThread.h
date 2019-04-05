#pragma once

#include <QThread>

class RPZCThread : public QThread {
    
    Q_OBJECT

    signals:
        void receivedMessage(const std::string message);
        void loggedUsersUpdated(QVariantList users);
        void historyReceived();
        void error(const std::string errMessage);
};