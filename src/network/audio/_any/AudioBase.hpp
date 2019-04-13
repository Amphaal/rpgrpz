#pragma once

#include <QUdpSocket>
#include <QString>

#include <QObject>

class AudioBase : public QObject {

    public:
        AudioBase(const QString &port) : _port(port.toInt()), _socket(new QUdpSocket(this)) { };

    protected:
        int _port;
        QUdpSocket* _socket;
};