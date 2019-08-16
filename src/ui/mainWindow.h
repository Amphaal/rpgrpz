#pragma once

#include "src/localization/i18n.h"
#include "src/helpers/_appContext.h"

#include "src/shared/commands/RPZActions.h"

#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QDesktopServices>
#include <QTabWidget>
#include <QLabel>
#include <QStatusBar>
#include <QApplication>

#include <QFileDialog>

#include "src/network/ConnectivityHelper.h"
#include "src/network/rpz/server/RPZServer.h"

#include "src/ui/communication/ConnectWidget.h"
#include "src/ui/communication/chat/ChatWidget.h"
#include "src/ui/communication/audio/AudioManager.h"
#include "src/ui/map/MapView.h"
#include "src/ui/map/MapTools.h"
#include "src/ui/layout/MapLayoutManager.h"

#include "src/ui/assets/AssetsManager.h"

#include "src/ui/atoms/AtomEditionManager.hpp"

#include "src/ui/others/RestoringSplitter.h"
#include "src/ui/others/UpdaterUIIntegrator.h"
#include "src/ui/others/statusbar/RPZStatusBar.h"

#include "src/ui/others/statusbar/RPZStatusLabel.h"

class MainWindow : public QMainWindow { 
    public:
        MainWindow();
        ~MainWindow();

    private:
        void _trueShow();
        void closeEvent(QCloseEvent *event) override;
        
        void _initUI();

        void _saveWindowState();
        void _loadWindowState();

        ConnectivityHelper* _ipHelper = nullptr;
        RPZServer _rpzServer;
        bool _mustLaunchServer = true;
        void _initConnectivity();

        MapView* _mapView = nullptr;
        ConnectWidget* _connectWidget = nullptr;
        AudioManager* _audioManager = nullptr;
        ChatWidget* _cw = nullptr;
        AssetsManager* _assetsManager = nullptr;
        MapTools* _mapTools = nullptr;
        MapLayoutManager* _mlManager = nullptr;
        AtomEditionManager* _atomEditManager = nullptr;
        void _initUIApp();

        /*statusbar*/
        RPZStatusBar* _sb = nullptr;
        void _initUIStatusBar();

        /*menu*/
        void _initUIMenu();
        QAction *cfugAction;
        QMenu* _getToolsMenu();
        QMenu* _getFileMenu();
        QMenu* _getHelpMenu();
        QMenu* _getMapMenu();
            void _saveRPZMapAs();
            void _loadRPZMap();

        UpdaterUIIntegrator* _updateIntegrator = nullptr;
};