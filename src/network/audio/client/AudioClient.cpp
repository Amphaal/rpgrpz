#include "AudioClient.h"

AudioClient::AudioClient(const QString &domain, const QString &port) : AudioBase(port) { 

    this->_socket->bind(QHostAddress::Any, this->_port);

    QObject::connect(
        this->_socket, &QUdpSocket::readyRead,
        this, &AudioClient::_onReceivedDatagram
    );
    
}

void AudioClient::_onReceivedDatagram() {
    
    //while datagrams waiting to be handled
    while (this->_socket->hasPendingDatagrams()) {

        //receive data
        auto datagram = this->_socket->receiveDatagram();
        if(!datagram.isValid()) continue;

        //extract data, inform bps counter
        auto data = datagram.data();
        this->_accountBpsIn(data.size());

        //TODO processTheDatagram

    }

}