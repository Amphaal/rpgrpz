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
#include <QDebug>

#include <nw-candy/uPnPHandler.h>

// .hpp for QT bindings to generate
class uPnPThread : public QThread, public NetworkCandy::uPnPHandler {
    Q_OBJECT

 public:
    uPnPThread(const QString &targetPort, const QString &description)
        : NetworkCandy::uPnPHandler(targetPort.toStdString(), description.toStdString()) {}

    void run() override {
        auto result = this->ensurePortMapping();
        if (!result) emit uPnPError();
        else  uPnPSuccess(
            QString::fromStdString(this->localIP()),
            QString::fromStdString(this->externalIP()),
            QString::fromStdString(NetworkCandy::uPnPHandler::PROTOCOL),
            QString::fromStdString(this->portToMap())
        );
    }

    ~uPnPThread() {
        this->mayDeletePortMapping();
    }

 signals:
    void uPnPError();
    void uPnPSuccess(const QString &localIP, const QString &extIP, const QString &protocol, const QString &negociatedPort);
};
