#pragma once

#include "src/localization/i18n.hpp"

#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QDesktopServices>
#include <QTabWidget>
#include <QLabel>
#include <QStatusBar>
#include <QGroupBox>
#include <QApplication>

#include "src/network/ConnectivityHelper.h"
#include "src/network/rpz/server/RPZServer.h"

#include "src/ui/communication/ConnectWidget.h"
#include "src/ui/communication/ChatWidget.h"
#include "src/ui/communication/audio/AudioStreamController.h"
#include "src/ui/communication/audio/PlaylistController.h"

#include "src/ui/map/MapView.h"
#include "src/ui/map/MapTools.h"
#include "src/ui/map/MapLayoutManager.h"

#include "src/ui/assets/AssetsManager.h"

#include "src/ui/others/RestoringSplitter.hpp"
#include "src/ui/others/UpdaterUIIntegrator.hpp"
#include "src/ui/others/RPZStatusBar.hpp"

class MainWindow : public QMainWindow { 
    public:
        MainWindow();

    private:
        void _trueShow();
        void closeEvent(QCloseEvent *event) override;
        
        void _initUI();

        ConnectivityHelper* _ipHelper;
        RPZServer* _rpzServer = nullptr;
        bool _mustLaunchServer = true;
        void _initConnectivity();

        MapView* _mapView = nullptr;
        ConnectWidget* _connectWidget;
        AudioStreamController* _streamNotifier;
        PlaylistController* _streamController;
        ChatWidget* _cw = nullptr;
        AssetsManager* _assetsManager;
        MapTools* _mapTools;
        MapLayoutManager* _mlManager;
        void _initUIApp();

        /*statusbar*/
        RPZStatusBar* _sb;
        void _initUIStatusBar();

        /*menu*/
        void _initUIMenu();
        QAction *cfugAction;
        QMenu* _getToolsMenu();
        QMenu* _getFileMenu();
        QMenu* _getHelpMenu();

        UpdaterUIIntegrator* _updateIntegrator = nullptr;
};