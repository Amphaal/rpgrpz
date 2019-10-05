#include "ClientBindable.h"

ClientBindable::ClientBindable() {
    _boundWidgets.append(this);
}

QVector<ClientBindable*> ClientBindable::boundWidgets() {
    return _boundWidgets;
}

void ClientBindable::bindAll(RPZClient* cc) {
    
    _rpzClient = cc;

    //on disconnect
    QObject::connect(
        _rpzClient->thread(), &QThread::finished,
        &ClientBindable::_onClientThreadFinished
    );
    
    //trigger connection
    for(auto ref : _boundWidgets) {
        ref->onRPZClientConnecting();
    }

}

void ClientBindable::_onClientThreadFinished() {
    _rpzClient = nullptr;
    QMetaObject::invokeMethod(ClientBindableMain::get(), "trigger");
}

void ClientBindable::unbindAll() {
    if(_rpzClient && _rpzClient->thread()->isRunning()) {
        _rpzClient->thread()->quit();
		_rpzClient->thread()->wait();
    }
}

void ClientBindable::onRPZClientConnecting() {}
void ClientBindable::onRPZClientDisconnect() {}