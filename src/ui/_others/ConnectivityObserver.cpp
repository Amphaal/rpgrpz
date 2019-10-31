#include "ConnectivityObserver.h"

ConnectivityObserver::ConnectivityObserver() {
    _observers.append(this);
}
        
void ConnectivityObserver::bindAll(RPZClient* cc) {
        
    _rpzClient = cc;

    QObject::connect(
        _rpzClient->thread(), &QThread::finished,
        &ConnectivityObserver::_onClientThreadFinished
    );

    //trigger connection
    for(auto ref : _observers) {
        ref->connectingToServer();
    }

}

void ConnectivityObserver::unbindAll() {
    if(_rpzClient && _rpzClient->thread()->isRunning()) {
        _rpzClient->thread()->quit();
        _rpzClient->thread()->wait();
    }
}

const QVector<ConnectivityObserver*> ConnectivityObserver::observers() {
    return _observers;
}

void ConnectivityObserver::receivedConnectionCloseSignal() {
    this->connectionClosed();
}

void ConnectivityObserver::_onClientThreadFinished() {
    _rpzClient = nullptr;
    RPZClient::resetHostAbility();
    QMetaObject::invokeMethod(ConnectivityObserverSynchronizer::get(), "triggerConnectionClosed");
}


ConnectivityObserverSynchronizer* ConnectivityObserverSynchronizer::get() {
    if(!_inst) _inst = new ConnectivityObserverSynchronizer;
    return _inst;
}

void ConnectivityObserverSynchronizer::triggerConnectionClosed() {
    for(auto observer : ConnectivityObserver::observers()) {
        observer->receivedConnectionCloseSignal();
    }
}
