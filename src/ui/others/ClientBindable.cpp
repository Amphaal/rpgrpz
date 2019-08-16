#include "ClientBindable.h"

ClientBindable::ClientBindable() {
    _boundWidgets.append(this);
}

void ClientBindable::bindAll(RPZClient* cc) {
    
    _rpzClient = cc;

    //on disconnect
    QObject::connect(
        _rpzClient, &QThread::finished,
		ClientBindable::_onClientThreadFinished
    );
    
    //trigger connection
    for(auto ref : _boundWidgets) {
        ref->onRPZClientConnecting();
    }

}

void ClientBindable::_onClientThreadFinished() {
    delete _rpzClient;
    _rpzClient = nullptr;

    for(auto ref : _boundWidgets) {
        ref->onRPZClientDisconnect();
    }
}

void ClientBindable::unbindAll() {
    if(_rpzClient && _rpzClient->isRunning()) {
        _rpzClient->exit();
		_rpzClient->wait();
    }
}

void ClientBindable::onRPZClientConnecting() {}
void ClientBindable::onRPZClientDisconnect() {}