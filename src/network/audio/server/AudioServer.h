#pragma once

#include <QSet>
#include <QHostAddress>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QTimer>

#include "../_any/AudioBase.hpp"
#include "../../rpz/_any/JSONSocket.h"

class AudioServer : public AudioBase {

    Q_OBJECT

    public:
        AudioServer(const QString &port);
    
    signals:
        void kbpsSent(double kbps, int clients);

    public slots:
        void sendAudio(const QString &pathToAudio);
        void addClient(JSONSocket* client);

    private:
        QSet<QHostAddress> _clients;
        qint64 _maxChunkSize = 1000;
        QTimer* _kbpsTimer;
        qint64 _bpsCountBuffer = 0;
        qint64 _lastBpsEmission;
        void emitKbpsSent();
};