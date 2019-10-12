#include "UpdaterUIIntegrator.h"

UpdaterUIIntegrator::UpdaterUIIntegrator(QMainWindow* wParent) : QObject(wParent), _wParent(wParent) {

    if(MAINTENANCE_TOOL_LOCATION == "") {
        this->_updater = new QtAutoUpdater::Updater(wParent);
    }
    else {
        this->_updater = new QtAutoUpdater::Updater(MAINTENANCE_TOOL_LOCATION, wParent);
    }

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
};

void UpdaterUIIntegrator::checkForAppUpdates() {
    emit stateChanged(true);
    this->_updater->checkForUpdates();
}

void UpdaterUIIntegrator::_onUpdateChecked(const bool hasUpdate, const bool hasError) {

    //if the user asks directly to check updates
    if(this->_userNotificationOnUpdateCheck) {
        this->_userNotificationOnUpdateCheck = false;
        
        const QString title = QString(APP_NAME) + " - " + tr("Check for updates...");
        const QString content = this->_updater->errorLog();

        if(!hasUpdate && !hasError) {
            QMessageBox::information(this->_wParent, 
                title, 
                content, 
                QMessageBox::Ok, QMessageBox::Ok);
        } else if (hasError) {
            QMessageBox::warning(this->_wParent, 
                title, 
                content, 
                QMessageBox::Ok, QMessageBox::Ok);
        }
    }

    //no update, no go
    if(!hasUpdate) {
        emit stateChanged(false);
        return;
    }

    //if has update
    const QString title = QString(APP_NAME) + " - " + tr("Update Available");
    const QString content = tr("An update is available for %1. Would you like to install it now ?").arg(APP_NAME);

    auto msgboxRslt = QMessageBox::information(this->_wParent, 
                title, 
                content, 
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
    );
    
    if(msgboxRslt == QMessageBox::Yes) {
        this->openMaintenanceTool();
    } else {
        emit stateChanged(false);
    }

};