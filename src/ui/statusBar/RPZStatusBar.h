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

#include <QStatusBar>
#include <QDebug>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>
#include <QFileInfo>
#include <QPushButton>

#include "RPZStatusLabel.h"
#include "ClientActivityBar.hpp"
#include "DownloadStatus.hpp"

#include "src/ui/_others/ConnectivityObserver.h"
#include "src/network/rpz/server/RPZServer.h"
#include "src/shared/hints/HintThread.hpp"
#include "src/ui/serverLogs/ServerLogs.hpp"

class RPZStatusBar : public QStatusBar, public ConnectivityObserver {
    Q_OBJECT

 public:
    explicit RPZStatusBar(QWidget * parent = nullptr);

    void setBoundServer(RPZServer* server);
    void updateServerState_NoServer();

 public slots:
    void updateUPnPLabel(const QString &stateText, RPZStatusLabel::State state);
    void updateExtIPLabel(const QString &stateText, RPZStatusLabel::State state);

 private slots:
    void _updateServerState_Listening();
    void _updateServerState_Failed();
    void _onServerIsActive();
    void _onServerIsInactive();
    void _updateMapFileLabel(const QString &mapDescriptor, bool isMapDirty);

 private:
    RPZStatusLabel* _extIpLabel = nullptr;
    RPZStatusLabel* _upnpStateLabel = nullptr;
    RPZStatusLabel* _serverStateLabel = nullptr;
    RPZStatusLabel* _mapFileLabel = nullptr;
    ClientActivityBar* _activityIndicators = nullptr;
    DownloadStatus* _dlStatus = nullptr;
    QPushButton* _serverLogsBtn = nullptr;
    ServerLogs* _serverLogs = nullptr;

    void _installComponents();
    void _installLayout();

    void connectingToServer() override;
    void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override;

    void _updateServerStateLabel(const QString &stateText, RPZStatusLabel::State state);
};
