#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QStackedWidget>

#include "src/helpers/_appContext.h"

#include "src/shared/commands/RPZActions.h"

#include "src/network/ConnectivityHelper.h"
#include "src/network/rpz/server/RPZServer.h"

#include "src/ui/logIn/ConnectWidget.h"
#include "src/ui/chat/ChatWidget.h"
#include "src/ui/audio/AudioManager.h"
#include "src/ui/map/MapView.h"

#include "src/ui/map/toolbars/MapTools.hpp"
#include "src/ui/map/toolbars/MapHelpers.hpp"
#include "src/ui/map/toolbars/MapActions.hpp"

#include "src/ui/mapLayout/MapLayoutManager.h"
#include "src/ui/users/players/PlayersListView.hpp"
#include "src/ui/users/standard/StandardUsersListView.hpp"

#include "src/ui/toysBox/ToysTreeView.h"

#include "src/ui/atomEditor/AtomEditionManager.hpp"

#include "src/ui/_others/RestoringSplitter.h"
#include "src/ui/_others/UpdaterUIIntegrator.h"
#include "src/ui/statusBar/RPZStatusBar.h"

#include "src/ui/statusBar/RPZStatusLabel.h"

#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/ui/sheets/CharacterEditor.hpp"

#include "src/ui/map/MiniMapView.hpp"
#include "src/ui/map/_base/OverlayingLayout.hpp"

class MainWindow : public QMainWindow, public ConnectivityObserver { 
    
    Q_OBJECT
    
    public:
        enum class UIMode { Unset, Full, Player };
        Q_ENUM(UIMode)

        MainWindow();
        ~MainWindow();
    
    protected:
        void connectingToServer() override;
        void connectionClosed(bool hasInitialMapLoaded) override;

    private slots:
        void _onConnectionToServer();

    private:
        void closeEvent(QCloseEvent *event) override;
        
        void _initUI();

        void _saveWindowState();
        void _loadWindowState();

        ConnectivityHelper* _ipHelper = nullptr;
        RPZServer* _rpzServer = nullptr;
        bool _mustLaunchServer = true;
        void _initConnectivity();

        void _initAppComponents();
            QWidget* _mapViewContainer = nullptr;
            MapView* _mapView = nullptr;
            MiniMapView* _minimap = nullptr;

            MapActions* _mapActions = nullptr;
            MapHelpers* _mapHelpers = nullptr;
            PlayersListView* _playersView = nullptr;
            StandardUsersListView* _usersView = nullptr;
            ConnectWidget* _connectWidget = nullptr;
            AudioManager* _audioManager = nullptr;
            ChatWidget* _chatWidget = nullptr;
            ToysTreeView* _toys = nullptr;
            MapTools* _mapTools = nullptr;
            MapLayoutManager* _mlManager = nullptr;
            AtomEditionManager* _atomEditManager = nullptr;
            CharacterEditor* _characterEditor = nullptr;

        void _initAppUnmovableUI();
        QTabWidget* _leftTab = nullptr;
        QTabWidget* _rightTab = nullptr;

        UIMode _currentAppUIMode = UIMode::Unset;
        const UIMode _defaultAppUIMode = UIMode::Full;
        void _setupAppUI(UIMode mode);

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
        
        bool _showBars = false;
        void _triggerBarsVisibility();
        void _barVisibilityToolTip();

        void keyPressEvent(QKeyEvent * event) override;
};