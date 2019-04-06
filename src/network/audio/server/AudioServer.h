#pragma once

#include <QUdpSocket>
#include <QString>

#include "ASThread.h"

class AudioServer : public ASThread {
    public:
        AudioServer(const QString &domain, const QString &port);
        void run() override;
        void startBroadcasting(const QString &localPathToMusic);

    private:
        QString _port;
        QString _name;
        QUdpSocket* _broadcastSocket;
};