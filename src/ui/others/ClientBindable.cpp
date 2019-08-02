#include "ClientBindable.h"

ClientBindable::ClientBindable() {
    _boundWidgets.append(this);
}

void ClientBindable::bindAll(RPZClientThread* cc) {
    for(auto widget : _boundWidgets) {
        widget->onRPZClientThreadConnecting(cc);
    }
}

void ClientBindable::unbindAll() {
    for(auto widget : _boundWidgets) {
        widget->unbindRPZClientThread();
    }
}

void ClientBindable::onRPZClientThreadConnecting(RPZClientThread* cc) {
    
    this->unbindRPZClientThread();
    this->_rpzClient = cc;

    //on disconnect
    QObject::connect(
        this->_rpzClient->socket(), &QAbstractSocket::disconnected,
        [&]() {
            this->onRPZClientThreadDisconnect(this->_rpzClient);
        }
    );

    //pass to nullptr for comparaisons
    QObject::connect(
        this->_rpzClient, &QObject::destroyed,
        [&]() {
            this->_rpzClient = nullptr;
        }
    );

}

void ClientBindable::onRPZClientThreadDisconnect(RPZClientThread* cc) {}

void ClientBindable::unbindRPZClientThread() {
    if(this->_rpzClient) {
        this->_rpzClient->disconnect();
    }
}
