#include "ClientBindable.h"

ClientBindable::ClientBindable() {
    _boundWidgets.append(this);
}

void ClientBindable::bindAll(RPZClient* cc) {
    for(auto widget : _boundWidgets) {
        widget->onRPZClientConnecting(cc);
    }
}

void ClientBindable::unbindAll() {
    for(auto widget : _boundWidgets) {
        widget->unbindRPZClient();
    }
}

void ClientBindable::onRPZClientConnecting(RPZClient* cc) {
    
    this->unbindRPZClient();
    this->_rpzClient = cc;

    //on disconnect
    QObject::connect(
        this->_rpzClient, &JSONSocket::disconnected,
        [&]() {
            this->onRPZClientDisconnect(this->_rpzClient);
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

void ClientBindable::onRPZClientDisconnect(RPZClient* cc) {}

void ClientBindable::unbindRPZClient() {
    if(this->_rpzClient) {
        this->_rpzClient->disconnect();
    }
}
