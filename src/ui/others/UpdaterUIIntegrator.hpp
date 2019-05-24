#pragma once

#include "src/localization/i18n.hpp"
#include "src/helpers/_appContext.h"

#include <QDebug>
#include <QMessageBox>
#include <QMainWindow>
#include "libs/qtautoupdater/autoupdatercore/updater.h"

class UpdaterUIIntegrator : public QObject {
    
    Q_OBJECT
    
    signals:
        void stateChanged(const bool isSearching);

    public:
        UpdaterUIIntegrator(QMainWindow* wParent) : QObject(wParent), _wParent(wParent) {
             
            qDebug() << "UI : AutoUpdate instantiation";

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

        void openMaintenanceTool() {
            this->_updater->runUpdaterOnExit();
            this->_wParent->close();
        }
    
    public slots:
        void requireUpdateCheckFromUser() {

            this->_userNotificationOnUpdateCheck = true;

            if (!this->_updater->isRunning()) {
                this->checkForAppUpdates();
            }
        };

        void checkForAppUpdates() {
            emit stateChanged(true);
            this->_updater->checkForUpdates();
        }

    private:
        QMainWindow* _wParent = nullptr;
        QtAutoUpdater::Updater* _updater;
        bool _userNotificationOnUpdateCheck = false;

        void _onUpdateChecked(const bool hasUpdate, const bool hasError) {

            //if the user asks directly to check updates
            if(this->_userNotificationOnUpdateCheck) {
                this->_userNotificationOnUpdateCheck = false;
                
                const QString title = QString(APP_NAME) + " - " + I18n::tr()->Menu_CheckForUpgrades();
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
            const QString title = QString(APP_NAME) + " - " + I18n::tr()->Alert_UpdateAvailable_Title();
            const QString content = I18n::tr()->Alert_UpdateAvailable_Text();

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
};