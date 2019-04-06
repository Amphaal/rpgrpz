#pragma once

#include <QThread>

class RPZCThread : public QThread {
    
    Q_OBJECT

    signals:
        void receivedMessage(const std::string &message);
        void loggedUsersUpdated(const QVariantList &users);
        void logHistoryReceived();
        void error(const std::string &errMessage);
        void hostMapChanged(const QVariantList &data);
};