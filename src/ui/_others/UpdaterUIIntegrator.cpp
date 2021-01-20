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
    this->_updater = new QtAutoUpdater::Updater(APP_MAINTENANCETOOL_PATH, wParent);

    QObject::connect(
        this->_updater, &QtAutoUpdater::Updater::checkUpdatesDone,
        this, &UpdaterUIIntegrator::_onUpdateChecked
    );
}

void UpdaterUIIntegrator::openMaintenanceTool() {
    this->_updater->runUpdaterOnExit();
    this->_wParent->close();
}


void UpdaterUIIntegrator::requireUpdateCheckFromUser() {
    this->_userNotificationOnUpdateCheck = true;

    if (!this->_updater->isRunning()) {
        this->checkForAppUpdates();
    }
}

void UpdaterUIIntegrator::checkForAppUpdates() {
    emit stateChanged(true);
    this->_updater->checkForUpdates();
}

void UpdaterUIIntegrator::_onUpdateChecked(const bool hasUpdate, const bool hasError) {
    // if the user asks directly to check updates
    if (this->_userNotificationOnUpdateCheck) {
        this->_userNotificationOnUpdateCheck = false;

        auto title = QString(APP_NAME) + " - " + tr("Check for updates...");
        auto content = QString::fromUtf8(this->_updater->errorLog());

        if (!hasUpdate && !hasError) {
            QMessageBox::information(
                this->_wParent,
                title,
                content,
                QMessageBox::Ok, QMessageBox::Ok
            );
        } else if (hasError) {
            QMessageBox::warning(
                this->_wParent,
                title,
                content,
                QMessageBox::Ok, QMessageBox::Ok
            );
        }
    }

    // no update, no go
    if (!hasUpdate) {
        emit stateChanged(false);
        return;
    }

    // if has update
    auto title = QString(APP_NAME) + " - " + tr("Update Available");
    auto content = tr("An update is available for %1. Would you like to install it now ?").arg(APP_NAME);

    auto msgboxRslt = QMessageBox::information(
        this->_wParent,
        title,
        content,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
    );

    if (msgboxRslt == QMessageBox::Yes) {
        this->openMaintenanceTool();
    } else {
        emit stateChanged(false);
    }
}
