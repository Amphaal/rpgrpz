#include "AudioServer.h"

AudioServer::AudioServer(const QString &port) : AudioBase(port) { 
    
    //complete signal data
    QObject::connect(
        this, &AudioBase::kbpsSent,
        [&](double kbps) {
            emit kbpsSent(kbps, this->_clients.size());
        }
    );

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