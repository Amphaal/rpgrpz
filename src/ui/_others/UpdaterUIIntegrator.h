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

#include "src/helpers/_appContext.h"

#include <QDebug>
#include <QMessageBox>
#include <QMainWindow>
#include "src/_libs/autoupdatercore/updater.h"

class UpdaterUIIntegrator : public QObject {
    
    Q_OBJECT
    
 signals:
        void stateChanged(const bool isSearching);

 public:
        UpdaterUIIntegrator(QMainWindow* wParent);
        void openMaintenanceTool();
    
 public slots:
        void requireUpdateCheckFromUser();
        void checkForAppUpdates();

 private:
        QMainWindow* _wParent = nullptr;
        QtAutoUpdater::Updater* _updater;
        bool _userNotificationOnUpdateCheck = false;

        void _onUpdateChecked(const bool hasUpdate, const bool hasError);
};