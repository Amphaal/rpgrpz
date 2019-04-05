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
#include <QDesktopServices>
#include <QTabWidget>


#include <QLabel>
#include <QStatusBar>
#include <QGroupBox>
#include <QApplication>

#include "src/network/ConnectivityHelper.h"
#include "src/network/rpz/server/RPZServer.h"

#include "src/ui/components/ConnectWidget.h"
#include "src/ui/components/MapView.h"
#include "src/ui/components/ChatWidget.h"
#include "src/ui/components/AssetsManager.h"
#include "src/ui/components/AudioStreamNotifier.h"
#include "src/ui/components/AudioStreamController.h"
#include "src/ui/components/MapTools.h"
#include "src/ui/components/RestoringSplitter.h"


class MainWindow : public QMainWindow { 
    public:
        MainWindow();

    private:
        void _trueShow();
        void closeEvent(QCloseEvent *event) override;
        
        void _initUI();

        ConnectivityHelper* _ipHelper;
        RPZServer* _chatServer = 0;
        void _initConnectivity();

        MapView* _mapView = 0;
        ConnectWidget* _connectWidget = 0;
        AudioStreamNotifier* _streamNotifier = 0;
        ChatWidget* _cw = 0;
        AssetsManager* _assetsManager = 0;
        MapTools* _mapTools = 0;
        void _initUIApp();

        /*statusbar*/
        QLabel* _localIpLabel;
        QLabel* _extIpLabel;
        QLabel* _upnpStateLabel;
        void updateUPnPLabel(std::string state);
        void updateExtIPLabel(std::string state, bool isOn);
        void updateIntIPLabel(std::string state);
        void _initUIStatusBar();

        /*menu*/
        void _initUIMenu();
        QAction *cfugAction;
        QMenu* _getAboutMenu();
        QMenu* _getFileMenu();
        QMenu* _getHelpMenu();

        /*auto updated*/
        QtAutoUpdater::Updater *updater;
        bool userNotificationOnUpdateCheck = false;
        void UpdateSearch_switchUI(bool isSearching);
        void _setupAutoUpdate();
        void onUpdateChecked(bool hasUpdate, bool hasError);
        void requireUpdateCheckFromUser();
        void checkForAppUpdates();
};