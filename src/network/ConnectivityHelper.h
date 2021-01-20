// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QDebug>
#include <QString>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>

#include "src/network/uPnPThread.hpp"
#include "src/network/CMThread.hpp"

#include "src/helpers/_appContext.h"

#include "src/ui/statusBar/RPZStatusLabel.h"

class ConnectivityHelper : public QObject {
    Q_OBJECT

 public:
    explicit ConnectivityHelper(QObject *parent = nullptr);
    ~ConnectivityHelper();

    void startWorking();

 signals:
    void remoteAddressStateChanged(const QString &stateText, RPZStatusLabel::State state = RPZStatusLabel::State::Finished);
    void uPnPStateChanged(const QString &stateText, RPZStatusLabel::State state = RPZStatusLabel::State::Finished);

 private:
    uPnPThread* _upnpThread = nullptr;
    CMThread* _cmThread = nullptr;

    const QString _getWaitingText() const;
    const QString _getErrorText() const;

    void _requestUPnPHandshake();
    void _mayClearUPnPThread();

    void _onUPnPSuccess(const QString &localIP, const QString &extIP, const QString &protocol, const QString &negociatedPort);
    void _onUPnPError();
    void _onConnectivityChanged(bool isConnectedToInternet);
};
