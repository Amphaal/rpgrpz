#include "ConnectivityObserver.h"

ConnectivityObserver::ConnectivityObserver() {
    _observers.append(this);
}
        
void ConnectivityObserver::connectWithClient(RPZClient* cc) {

    //prevent if client exists
    if(_rpzClient) return;

    _rpzClient = cc;

    //create a separate thread to run the client into
    auto clientThread = new QThread;
    clientThread->setObjectName(QStringLiteral(u"RPZClient Thread"));
    _rpzClient->moveToThread(clientThread);
    
    //events...
    QObject::connect(
        clientThread, &QThread::started, 
        _rpzClient, &RPZClient::run
    );

    QObject::connect(
        _rpzClient, &RPZClient::closed, 
        clientThread, &QThread::quit
    );

    QObject::connect(
        _rpzClient->thread(), &QThread::finished,
        &ConnectivityObserver::_onClientThreadFinished
    );

    //allow connection bindings on UI
    for(const auto ref : _observers) {
        ref->connectingToServer();
    }

    //start
    clientThread->start();

}

void ConnectivityObserver::disconnectClient() {
    if(_rpzClient && _rpzClient->thread()->isRunning()) {
        _rpzClient->thread()->quit();
    }
}

const QVector<ConnectivityObserver*> ConnectivityObserver::observers() {
    return _observers;
}

void ConnectivityObserver::receivedConnectionCloseSignal(bool hasInitialMapLoaded) {
    this->connectionClosed(hasInitialMapLoaded);
}

void ConnectivityObserver::_onClientThreadFinished() {

    auto hasInitialMapLoaded = _rpzClient->hasReceivedInitialMap();

    _rpzClient->thread()->deleteLater();
    _rpzClient->deleteLater();
    _rpzClient = nullptr;
    
    RPZClient::resetHostAbility();

    QMetaObject::invokeMethod(ConnectivityObserverSynchronizer::get(), "triggerConnectionClosed", 
        Q_ARG(bool, hasInitialMapLoaded)
    );

}


ConnectivityObserverSynchronizer* ConnectivityObserverSynchronizer::get() {
    if(!_inst) _inst = new ConnectivityObserverSynchronizer;
    return _inst;
}

void ConnectivityObserverSynchronizer::triggerConnectionClosed(bool hasInitialMapLoaded) {
    for(const auto observer : ConnectivityObserver::observers()) {
        observer->receivedConnectionCloseSignal(hasInitialMapLoaded);
    }
}
