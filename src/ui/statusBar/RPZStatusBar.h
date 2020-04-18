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

#include <QStatusBar>
#include <QDebug>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>
#include <QFileInfo>

#include "RPZStatusLabel.h"
#include "ClientActivityBar.hpp"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/ui/_others/ConnectivityObserver.h"
#include "DownloadStatus.hpp"

class RPZStatusBar : public QStatusBar, public ConnectivityObserver {

    Q_OBJECT

 public:
        RPZStatusBar(QWidget * parent = nullptr);

        void bindServerIndicators();

    public slots:
        void updateServerStateLabel(const QString &stateText, RPZStatusLabel::State state);
        void updateUPnPLabel(const QString &stateText, RPZStatusLabel::State state);
        void updateExtIPLabel(const QString &stateText, RPZStatusLabel::State state);
        void updateMapFileLabel(const QString &mapDescriptor, bool isMapDirty);

 private:
        RPZStatusLabel* _extIpLabel = nullptr;
        RPZStatusLabel* _upnpStateLabel = nullptr;
        RPZStatusLabel* _serverStateLabel = nullptr;
        RPZStatusLabel* _mapFileLabel = nullptr;
        ClientActivityBar* _activityIndicators = nullptr;
        DownloadStatus* _dlStatus = nullptr;

        void _installComponents();
        void _installLayout();

        virtual void connectingToServer() override;
        virtual void connectionClosed(bool hasInitialMapLoaded) override;

}; 