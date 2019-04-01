#pragma once

#include <QUdpSocket>
#include <QString>

#include "ASThread.h"

class AudioServer : public ASThread {
    public:
        AudioServer(QString domain, QString port);
        void run() override;
        void startBroadcasting(QString localPathToMusic);

    private:
        QString _port;
        QString _name;
        QUdpSocket* _broadcastSocket;
};