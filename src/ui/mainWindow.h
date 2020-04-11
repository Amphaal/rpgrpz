// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

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
#include "src/ui/audio/PlaylistAudioManager.h"
#include "src/ui/map/MapView.h"
#include "src/ui/map/_base/MapViewInteractibleDescriptor.hpp"

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
        void _onGameSessionReceived(const RPZGameSession &gameSession);
        void _onCharacterFocusRequest(const RPZCharacter::Id &characterIdToFocus);

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
            MapViewInteractibleDescriptor* _interactibleDescr = nullptr;

            MapActions* _mapActions = nullptr;
            MapHelpers* _mapHelpers = nullptr;
            PlayersListView* _playersView = nullptr;
            StandardUsersListView* _usersView = nullptr;
            ConnectWidget* _connectWidget = nullptr;
            PlaylistAudioManager* _audioManager = nullptr;
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
        
        void _triggerBarsVisibility();
        void _barVisibilityToolTip();

        void keyPressEvent(QKeyEvent * event) override;
};