#pragma once

#include "../_any/AudioBase.hpp"

#include <QByteArray>
#include <QNetworkDatagram>

class AudioClient : public AudioBase { 
    public:
        AudioClient(const QString &domain, const QString &port);

    private:
        void _onReceivedDatagram();
};