#pragma once

#include "src/localization/i18n.h"
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