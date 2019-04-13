#include "AudioClient.h"

AudioClient::AudioClient(const QString &domain, const QString &port) : AudioBase(port) { 

    this->_socket->bind(QHostAddress::Any, this->_port);

    QObject::connect(
        this->_socket, &QUdpSocket::readyRead,
        this, &AudioClient::_onReceivedDatagram
    );
    
}

void AudioClient::_onReceivedDatagram() {
    while (this->_socket->hasPendingDatagrams()) {
        auto datagram = this->_socket->receiveDatagram();
        //processTheDatagram(datagram);
    }
}