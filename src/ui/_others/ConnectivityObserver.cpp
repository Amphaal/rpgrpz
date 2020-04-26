// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "ConnectivityObserver.h"

ConnectivityObserver::ConnectivityObserver() {
    _observers.append(this);
}

void ConnectivityObserver::connectWithClient(RPZClient* cc) {
    // prevent if client exists
    if (_rpzClient) return;

    _rpzClient = cc;

    // create a separate thread to run the client into
    auto clientThread = new QThread;
    clientThread->setObjectName(QStringLiteral(u"RPZClient Thread"));
    _rpzClient->moveToThread(clientThread);

    // events...
    QObject::connect(
        clientThread, &QThread::started,
        _rpzClient, &RPZClient::run
    );

    QObject::connect(
        _rpzClient, &RPZClient::ended,
        ConnectivityObserverSynchronizer::get(), &ConnectivityObserverSynchronizer::onClientEnded
    );

    QObject::connect(
        _rpzClient->thread(), &QThread::finished,
        [&](){
            _rpzClient = nullptr;
        }
    );

    // allow connection bindings on UI
    for (const auto ref : _observers) {
        ref->connectingToServer();
    }

    // start
    clientThread->start();
}

void ConnectivityObserver::disconnectClient() {
    if (!_rpzClient || !_rpzClient->thread()->isRunning()) return;
    QMetaObject::invokeMethod(_rpzClient, "quit");
}

const QVector<ConnectivityObserver*> ConnectivityObserver::observers() {
    return _observers;
}

void ConnectivityObserver::receivedConnectionCloseSignal(bool hasInitialMapLoaded, const QString &errorMessage) {
    this->connectionClosed(hasInitialMapLoaded, errorMessage);
}

ConnectivityObserverSynchronizer* ConnectivityObserverSynchronizer::get() {
    if (!_inst) _inst = new ConnectivityObserverSynchronizer;
    return _inst;
}

void ConnectivityObserverSynchronizer::onClientEnded(const QString &errorMessage) {
    auto client = dynamic_cast<RPZClient*>(this->sender());

    auto hasInitialMapLoaded = client->hasReceivedInitialMap();

    client->thread()->deleteLater();
    client->deleteLater();

    Authorisations::resetHostAbility();

    for (const auto observer : ConnectivityObserver::observers()) {
        observer->receivedConnectionCloseSignal(hasInitialMapLoaded, errorMessage);
    }
}
