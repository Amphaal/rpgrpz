#pragma once

#include <QUdpSocket>
#include <QString>

#include "ACThread.h"

class AudioClient : public ACThread {
    public:
        AudioClient(const QString &domain, const QString &port);
        void run() override;

    private:
        QString _port;
        QString _name;
        QUdpSocket* _listeningSocket;
};