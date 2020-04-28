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

#pragma once

#include <QThread>

#include "src/network/rpz/server/RPZServer.h"

class ServerHosted {
 public:
    static void start() {
        _rpzServer = new RPZServer;

        // tell the UI when the server is down
        QObject::connect(
            _rpzServer, &RPZServer::failed,
            [&]() {
                _rpzServer = nullptr;
        });

        // create a separate thread to run the server into
        auto serverThread = new QThread;
        serverThread->setObjectName(QStringLiteral(u"RPZServer Thread"));
        _rpzServer->moveToThread(serverThread);

        // events...
        QObject::connect(
            serverThread, &QThread::started,
            _rpzServer, &RPZServer::run
        );

        QObject::connect(
            _rpzServer, &RPZServer::failed,
            serverThread, &QThread::quit
        );

        QObject::connect(
            serverThread, &QThread::finished,
            _rpzServer, &QObject::deleteLater
        );

        QObject::connect(
            serverThread, &QThread::finished,
            serverThread, &QObject::deleteLater
        );

        // start
        serverThread->start();
    }

    static void stop() {
        if (!_rpzServer) return;
        _rpzServer->thread()->quit();
        _rpzServer->thread()->wait();
    }

    static RPZServer* instance() {
        return _rpzServer;
    }

 private:
    static inline RPZServer* _rpzServer = nullptr;
};
