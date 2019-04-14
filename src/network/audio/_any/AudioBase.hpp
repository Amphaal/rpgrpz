#pragma once

#include <QUdpSocket>
#include <QString>
#include <QIODevice>
#include <QTimer>
#include <QObject>

class AudioBase : public QObject {

    Q_OBJECT

    signals:
        void kbpsSent(double kbps);
        void kbpsReceived(double kbps);

    public:
        AudioBase(const QString &port) : _port(port.toInt()), _socket(new QUdpSocket(this)), _kbpsTimer(new QTimer) { 
            
            //update count buffer at every successful writing
            QObject::connect(
                this->_socket, &QIODevice::bytesWritten,
                this, &AudioBase::_accountBpsOut
            );

            //every second, send kbps in
            QObject::connect(
                this->_kbpsTimer, &QTimer::timeout,
                this, &AudioBase::_emitKbpsOut
            );

            //every second, send kbps out
            QObject::connect(
                this->_kbpsTimer, &QTimer::timeout,
                this, &AudioBase::_emitKbpsIn
            );

            this->_kbpsTimer->setInterval(1000);
            this->_kbpsTimer->start();

        };

    protected:
        int _port;
        QUdpSocket* _socket;

        void _emitKbpsOut() {
            if(this->_bpsOutCountBuffer == this->_lastBpsOut) return;
            
            auto kbps = (double)this->_bpsOutCountBuffer / 1000;
            emit kbpsSent(kbps);

            this->_lastBpsOut = this->_bpsOutCountBuffer;
            this->_bpsOutCountBuffer = 0;
        };

        void _emitKbpsIn() {
            if(this->_bpsInCountBuffer == this->_lastBpsIn) return;
            
            auto kbps = (double)this->_bpsInCountBuffer / 1000;
            emit kbpsReceived(kbps);

            this->_lastBpsIn = this->_bpsInCountBuffer;
            this->_bpsInCountBuffer = 0;
        };

        void _accountBpsIn(qint64 bytes) {
            this->_bpsInCountBuffer += bytes;
        };

    private:
        QTimer* _kbpsTimer;
        
        qint64 _bpsInCountBuffer = 0;
        qint64 _lastBpsIn;

        qint64 _bpsOutCountBuffer = 0;
        qint64 _lastBpsOut;

        void _accountBpsOut(qint64 bytes) {
            this->_bpsOutCountBuffer += bytes;
        };

};