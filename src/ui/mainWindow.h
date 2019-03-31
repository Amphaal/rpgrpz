#pragma once

#include "src/localization/i18n.cpp"
#include "libs/qtautoupdater/autoupdatercore/updater.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QGraphicsItem>
#include <QClipboard>
#include <QToolTip>
#include <QGraphicsScene>

#include <QLabel>
#include <QStatusBar>
#include <QGroupBox>
#include <QApplication>

#include "src/network/ConnectivityHelper.h"
#include "src/network/chat/server/ChatServer.h"
#include "src/ui/components/ConnectWidget.h"

#include "src/ui/components/MapView.cpp"

#include "src/ui/components/ChatWidget.h"


class MainWindow : public QMainWindow { 
    public:
        MainWindow();

    private:
        void trueShow();

        void updateUPnPLabel(std::string state);
        void updateExtIPLabel(std::string state, bool isOn);
        void updateIntIPLabel(std::string state);

        ConnectivityHelper* _ipHelper;
        void _initUI();
        void _initConnectivity();

        MapView* _mapView;
        void _initMapView();

        ConnectWidget* _connectWidget = 0;
        void _initConnectionPanel();
        
        ChatWidget* _cw = 0;
        void _initChat();
        void onNewConnectionFromServer(std::string clientIp);

        void _initUIMenu();
        
        QLabel* _localIpLabel;
        QLabel* _extIpLabel;
        QLabel* _upnpStateLabel;
        void _initUIStatusBar();

        QAction *versionAction;
        QAction *cfugAction;

        QMenu* _getAboutMenu();
        QMenu* _getFileMenu();

        QtAutoUpdater::Updater *updater;
        bool userNotificationOnUpdateCheck = false;
        void UpdateSearch_switchUI(bool isSearching);
        void _setupAutoUpdate();
        void onUpdateChecked(bool hasUpdate, bool hasError);
        void requireUpdateCheckFromUser();;
        void checkForAppUpdates();
};