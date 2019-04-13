#include "AudioServer.h"

AudioServer::AudioServer(const QString &port) : AudioBase(port), _kbpsTimer(new QTimer) { 

    //update count buffer at every successful writing
    QObject::connect(
        this->_socket, &QIODevice::bytesWritten,
        [&](qint64 bytes) {
            this->_bpsCountBuffer += bytes;
        }
    );

    //every second, send kbps
    QObject::connect(
        this->_kbpsTimer, &QTimer::timeout,
        this, &AudioServer::emitKbpsSent
    );
    this->_kbpsTimer->setInterval(1000);
    this->_kbpsTimer->start();

};

void AudioServer::addClient(JSONSocket* client) {
    
    //add client
    auto newClientAddress = client->socket()->peerAddress();
    this->_clients.insert(newClientAddress);

    //auto remove from client list when disconnected
    QObject::connect(
        client, &JSONSocket::disconnected,
        [&]() {
            this->_clients.remove(newClientAddress);
        }
    );

}

void AudioServer::emitKbpsSent() {
    if(!this->_bpsCountBuffer == this->_lastBpsEmission) return;
    
    emit kbpsSent(this->_bpsCountBuffer / 1000, this->_clients.size());

    this->_lastBpsEmission = this-> _bpsCountBuffer;
    this-> _bpsCountBuffer= 0;
}

void AudioServer::sendAudio(const QString &pathToAudio) {

    //define the file
    QFile audioFile(pathToAudio);

    //stream...
    while(!audioFile.atEnd()) {
        
        auto chunk = audioFile.read(this->_maxChunkSize);

        //send it for each client
        for(auto &client : this->_clients) {
            this->_socket->writeDatagram(chunk, client, this->_port);
        }
    
    }
};