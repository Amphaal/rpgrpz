#pragma once

#include <QUdpSocket>
#include <QString>

#include "ACThread.h"

class AudioClient : public ACThread {
    public:
        AudioClient(QString domain, QString port);
        void run() override;

    private:
        QString _port;
        QString _name;
        QUdpSocket* _listeningSocket;
};