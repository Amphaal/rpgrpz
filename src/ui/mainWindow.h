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
#include <QShortcut>
#include <QToolTip>

#include <QFileDialog>

#include "src/network/ConnectivityHelper.h"
#include "src/network/rpz/server/RPZServer.h"

#include "src/ui/logIn/ConnectWidget.h"
#include "src/ui/chat/ChatWidget.h"
#include "src/ui/audio/AudioManager.h"
#include "src/ui/map/MapView.h"
#include "src/ui/map/MapTools.h"
#include "src/ui/mapLayout/MapLayoutManager.h"
#include "src/ui/users/UsersListView.hpp"

#include "src/ui/assets/AssetsManager.h"

#include "src/ui/atomEditor/AtomEditionManager.hpp"

#include "src/ui/_others/RestoringSplitter.h"
#include "src/ui/_others/UpdaterUIIntegrator.h"
#include "src/ui/statusBar/RPZStatusBar.h"

#include "src/ui/statusBar/RPZStatusLabel.h"

#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/ui/sheets/CharacterEditor.hpp"

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
        RPZServer* _rpzServer = nullptr;
        bool _mustLaunchServer = true;
        void _initConnectivity();

        UsersListView* _usersView = nullptr;
        MapView* _mapView = nullptr;
        ConnectWidget* _connectWidget = nullptr;
        AudioManager* _audioManager = nullptr;
        ChatWidget* _cw = nullptr;
        AssetsManager* _assetsManager = nullptr;
        MapTools* _mapTools = nullptr;
        MapLayoutManager* _mlManager = nullptr;
        AtomEditionManager* _atomEditManager = nullptr;
        CharacterEditor* _characterEditor = nullptr;
        void _initUIApp();

        /*statusbar*/
        RPZStatusBar* _sb = nullptr;
        void _initUIStatusBar();

        /*menu*/
        void _initUIMenu();
        QAction *cfugAction = nullptr;
        QMenu* _getToolsMenu();
        QMenu* _getFileMenu();
        QMenu* _getHelpMenu();
        QMenu* _getMapMenu();

        UpdaterUIIntegrator* _updateIntegrator = nullptr;

        void keyPressEvent(QKeyEvent * event) override;
        
        bool _showBars = false;
        void _triggerBarsVisibility();
        void _barVisibilityToolTip();
};