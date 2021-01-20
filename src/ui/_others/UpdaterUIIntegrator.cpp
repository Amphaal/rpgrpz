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

#include "UpdaterUIIntegrator.h"

UpdaterUIIntegrator::UpdaterUIIntegrator(QMainWindow* wParent) : QObject(wParent), _wParent(wParent) {
    this->_updater = QtAutoUpdater::Updater::create("qtifw", {
            {"path", APP_MAINTENANCETOOL_PATH}
        }, wParent);

    QObject::connect(
        this->_updater, &QtAutoUpdater::Updater::checkUpdatesDone,
        this, &UpdaterUIIntegrator::_onUpdateChecked
    );
}

void UpdaterUIIntegrator::openMaintenanceTool() {
    this->_updater->runUpdater(QtAutoUpdater::Updater::InstallModeFlag::OnExit);
    this->_wParent->close();
}


void UpdaterUIIntegrator::requireUpdateCheckFromUser() {
    this->_userNotificationOnUpdateCheck = true;

    if (!this->_updater->isRunning()) {
        this->checkForAppUpdates();
    }
}

void UpdaterUIIntegrator::checkForAppUpdates() {
    emit updateSeekingChanged(true);
    this->_updater->checkForUpdates();
}

void UpdaterUIIntegrator::_onUpdateChecked(QtAutoUpdater::Updater::State result) {
    // if the user asks directly to check updates
    bool mustNotifyNonProceeding = true;
    if (this->_userNotificationOnUpdateCheck) {
        this->_userNotificationOnUpdateCheck ^= true;
        mustNotifyNonProceeding ^= true;
    }

    // prepare
    bool proceed = false;
    auto title = tr("%1 - Check for updates...").arg(APP_NAME);
    
    // cases
    switch(result) {
        case QtAutoUpdater::Updater::State::NoUpdates: {
            if(mustNotifyNonProceeding) QMessageBox::information(this->_wParent,
                title,
                tr("No updates available."),
                QMessageBox::Ok,
                QMessageBox::Ok
            );
        }
        break;

        case QtAutoUpdater::Updater::State::Error: {
            if(mustNotifyNonProceeding) QMessageBox::information(this->_wParent,
                title,
                tr("Error while fetching updates !"),
                QMessageBox::Ok,
                QMessageBox::Ok
            );
        }
        break;

        case QtAutoUpdater::Updater::State::NewUpdates : {
            proceed = true;
        }
    }

    // no update, no go
    if (!proceed) {
        emit updateSeekingChanged(false);
        return;
    }

    // if has update
    auto msgboxRslt = QMessageBox::information(this->_wParent,
        tr("%1 - Update Available").arg(APP_NAME),
        tr("An update is available for %1. Would you like to install it now ?").arg(APP_NAME),
        QMessageBox::Yes | QMessageBox::No, 
        QMessageBox::Yes
    );

    // determine behavior
    if (msgboxRslt == QMessageBox::Yes) {
        this->openMaintenanceTool();
    } else {
        emit updateSeekingChanged(false);
    }
}
