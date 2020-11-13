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

#include <QDebug>
#include <QString>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QNetworkSession>
#include <QNetworkAccessManager>
#include <QNetworkConfigurationManager>

#include "src/network/uPnP/uPnPRequester.hpp"

#include "src/helpers/_appContext.h"

#include "src/ui/statusBar/RPZStatusLabel.h"

class ConnectivityHelper : public QObject {
    Q_OBJECT

 public:
    explicit ConnectivityHelper(QObject *parent = nullptr);
    ~ConnectivityHelper();
    void init();

 signals:
    void localAddressStateChanged(const QString &stateText, RPZStatusLabel::State state = RPZStatusLabel::State::Finished);
    void remoteAddressStateChanged(const QString &stateText, RPZStatusLabel::State state = RPZStatusLabel::State::Finished);
    void uPnPStateChanged(const QString &stateText, RPZStatusLabel::State state = RPZStatusLabel::State::Finished);

 private:
    QNetworkAccessManager* _nam = nullptr;
    QNetworkConfigurationManager* _ncm = nullptr;
    uPnPRequester* _upnpThread = nullptr;

    static inline QString _DebugStringModel = R"(Connectivity : %1 >> %2 [state:%3, type:%4, bearer:%5])";
    void _SSDebugNetworkConfig(const QString &descr, const QNetworkConfiguration &config);
    void _debugNetworkConfig();

    QString _getWaitingText();
    QString _getErrorText();

    void _getLocalAddress();
    void _tryNegociateUPnPPort();
    void _clearUPnPRequester();
    void _pickPreferedConfiguration();

    void _onUPnPSuccess(const QString &protocol, const QString &negociatedPort);
    void _onUPnPError();
    QString _upnp_extIp;
    void _onUPnPExtIpFound(const QString &extIp);
    void networkChanged(const QNetworkAccessManager::NetworkAccessibility accessible);

    void _mustReInit(const QNetworkConfiguration &config);

    QList<QNetworkConfiguration> _getDefinedConfiguration();
};
