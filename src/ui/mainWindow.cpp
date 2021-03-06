// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "mainWindow.h"

MainWindow::MainWindow() : _updateIntegrator(new UpdaterUIIntegrator(this)) {
    // start hint thread
    HintThread::init();

    // bind AlterationHandler / ConnectivityObserverOchestrator to UI Thread
    AlterationHandler::get();
    ConnectivityObserverSynchronizer::get();

    //
    AppContext::definePPcm(this);

    // init databases
    AssetsDatabase::get();
    CharactersDatabase::get();

    // init...
    this->_initUI();
    this->_initConnectivity();

    // initial show
    this->_loadWindowState();
    this->_triggerBarsVisibility();

    // load default map
    QMetaObject::invokeMethod(HintThread::hint(), "loadDefaultRPZMap");

    // start the update check
    this->_updateIntegrator->checkForAppUpdates();
}

MainWindow::~MainWindow() {
    // unbind network client from ui
    ConnectivityObserver::shutdownClient();

    // stop server if hosted on client
    ServerHosted::stop();
}

void MainWindow::connectingToServer() {
    QObject::connect(
        _rpzClient, &RPZClient::gameSessionReceived,
        this, &MainWindow::_onGameSessionReceived
    );
}

void MainWindow::_onGameSessionReceived(const RPZGameSession &gameSession) {
    Q_UNUSED(gameSession)
    auto mode = Authorisations::isHostAble() ? UIMode::Full : UIMode::Player;
    this->_setupAppUI(mode);
}

void MainWindow::connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) {
    this->_setupAppUI(_defaultAppUIMode);
}

void MainWindow::keyPressEvent(QKeyEvent * event) {
    switch (event->key()) {
        // unselect whatever is selected
        case Qt::Key::Key_Escape: {
            this->_mapView->resetTool();
            this->_audioManager->player()->playlist()->clearSelection();
            this->_toys->clearSelection();
        }
        break;

        default:
        break;
    }
}

void MainWindow::_triggerBarsVisibility() {
    auto areShown = AppContext::settings()->mainWindowBarsShown();
    this->menuBar()->setVisible(areShown);
    this->statusBar()->setVisible(areShown);
}

void MainWindow::_saveWindowState() {
    AppContext::settings()->beginGroup(QStringLiteral(u"mainWindow"));
    AppContext::settings()->setValue(QStringLiteral(u"windowGeometry"), this->saveGeometry());
    AppContext::settings()->setValue(QStringLiteral(u"windowState"), this->saveState());
    AppContext::settings()->endGroup();
}

void MainWindow::_loadWindowState() {
    // default state to save
    if (!AppContext::settings()->childGroups().contains(QStringLiteral(u"mainWindow"))) {
        this->showMaximized();
        this->_saveWindowState();
        return;
    }

    // load...
    AppContext::settings()->beginGroup(QStringLiteral(u"mainWindow"));
        this->restoreGeometry(
            AppContext::settings()->value(QStringLiteral(u"windowGeometry")).toByteArray()
        );
        this->restoreState(
            AppContext::settings()->value(QStringLiteral(u"windowState")).toByteArray()
        );
    AppContext::settings()->endGroup();

    this->show();
}

// handle clean close
void MainWindow::closeEvent(QCloseEvent *event) {
    // may save map changes
    HintThread::hint()->mayWantToSavePendingState(this);

    // save window state
    this->_saveWindowState();

    // normal close
    QMainWindow::closeEvent(event);
}

void MainWindow::_initConnectivity() {
    // check if we must use server capabilities
    auto appArgs = AppContext::getOptionArgs(QApplication::instance());
    if (appArgs.contains(QStringLiteral(u"noServer"))) {
        this->_mustLaunchServer = false;
        qDebug() << "RPZServer : No server to start because the user said so.";
        this->_sb->updateServerState_NoServer();
    }

    ////////////////////////////
    /// Connectivity helper ! //
    ////////////////////////////

    this->_ipHelper = new ConnectivityHelper(this);

    QObject::connect(
        this->_ipHelper, &ConnectivityHelper::remoteAddressStateChanged,
        this->_sb, &RPZStatusBar::updateExtIPLabel
    );
    QObject::connect(
        this->_ipHelper, &ConnectivityHelper::uPnPStateChanged,
        this->_sb, &RPZStatusBar::updateUPnPLabel
    );

    this->_ipHelper->startWorking();

    ////////////////////
    /// RPZServer ! //
    ////////////////////

    // may launch server
    if (this->_mustLaunchServer) {
        ServerHosted::start();
        this->_sb->setBoundServer(ServerHosted::instance());
    }
}


//////////////
/// UI init //
//////////////

void MainWindow::_initUI() {
    this->setWindowIcon(QIcon(QStringLiteral(u":/icons/app/app_32.png")));

    // central widget
    auto centralW = new RestoringSplitter("MainWindowSplitter");
    centralW->setContentsMargins(5, 5, 5, 5);
    this->setCentralWidget(centralW);

    // specific componements
    this->_initUIStatusBar();
    this->_initAppComponents();
    this->_initAppUnmovableUI();
    this->_setupAppUI(_defaultAppUIMode);
    this->_initUIMenu();

    // visibility
    this->_triggerBarsVisibility();

    // default focus
    this->_mapView->setFocus();
}

void MainWindow::_initAppUnmovableUI() {
    // left tabs
    this->_leftTab = new QTabWidget(this);
    this->_leftTab->addTab(this->_toys, QIcon(QStringLiteral(u":/icons/app/tabs/box.png")), tr("Toy box"));

    // right tabs
    this->_rightTab = new QTabWidget(this);
    this->_rightTab->setLayoutDirection(Qt::LayoutDirection::RightToLeft);

    auto chatLogWidget = [=]() {
        auto logLayout = new QVBoxLayout;
        logLayout->addWidget(this->_connectWidget);
        logLayout->addWidget(this->_usersView);
        logLayout->addWidget(this->_chatWidget, 1);

        auto CCwLayout = new QHBoxLayout;
        CCwLayout->setDirection(QBoxLayout::RightToLeft);
        CCwLayout->addLayout(logLayout, 1);
        CCwLayout->addWidget(this->_playersView);

        auto CCw = new QWidget(this);
        CCw->setLayout(CCwLayout);
        return CCw;
    }();

    auto gameHubTabIndex = this->_rightTab->addTab(chatLogWidget, QIcon(QStringLiteral(u":/icons/app/tabs/chat.png")), tr("Game Hub"));
    this->_rightTab->addTab(this->_docShareManager, QIcon(QStringLiteral(u":/icons/app/tabs/fileShare.png")), tr("Documents Share"));

    // alert messageLog for notifications
    auto updateUserVisble = [=](int currentIndex = 0) {
        auto isGameHubVisible = this->_rightTab->currentIndex() == gameHubTabIndex;
        this->_chatWidget->messageLog()->setIsUserVisible(isGameHubVisible);
    };
    updateUserVisble();
    QObject::connect(
        this->_rightTab, &QTabWidget::currentChanged,
        updateUserVisble
    );

    // update gamehub tab name
    QObject::connect(
        this->_chatWidget->messageLog(), &MessagesLog::notificationCountUpdated,
        [=](int newCount) {
            auto txt = tr("Game Hub");
            if (newCount) txt = QString("(%1) ").arg(newCount) + txt;
            this->_rightTab->setTabText(gameHubTabIndex, txt);
    });

    auto designerWidget = [=]() {
        auto designer = new QWidget(this);
        auto layout = new QVBoxLayout;
        designer->setLayout(layout);
        layout->setMargin(0);
        layout->setSpacing(2);

            auto toolbar = new QWidget(this);
            auto toolbarLayout = new QHBoxLayout;
            toolbar->setLayout(toolbarLayout);
            toolbarLayout->setMargin(0);
            toolbarLayout->setSpacing(0);

            toolbarLayout->addWidget(this->_mapHelpers);
            toolbarLayout->addStretch(0);
            toolbarLayout->addWidget(this->_mapTools);
            toolbarLayout->addWidget(this->_mapActions);
            toolbarLayout->addWidget(this->_getMenuToolbar());

        layout->addWidget(toolbar, 0, Qt::AlignTop);
        layout->addWidget(this->_mapViewContainer, 1);

        return designer;
    }();

    // final
    auto centralWidget = (RestoringSplitter*)this->centralWidget();
    centralWidget->addWidget(this->_leftTab);
    centralWidget->addWidget(designerWidget);
    centralWidget->addWidget(this->_rightTab);
    centralWidget->setStretchFactor(0, 0);
    centralWidget->setStretchFactor(1, 1);
    centralWidget->setStretchFactor(2, 0);
    centralWidget->restore();
}

void MainWindow::_setupAppUI(UIMode mode) {
    if (mode == this->_currentAppUIMode) return;

    auto removeFromTab = [](QTabWidget* tab, QWidget* toRemove) {
        auto index = tab->indexOf(toRemove);
        if (index == -1) return;
        tab->removeTab(index);
    };

    auto addCharacterEditor = [=](QTabWidget* tab) {tab->addTab(this->_characterEditor, QIcon(QStringLiteral(u":/icons/app/tabs/scroll.png")), tr("Sheets"));};
    auto addPlaylistAudioManager = [=](QTabWidget* tab) { tab->addTab(this->_audioManager, QIcon(QStringLiteral(u":/icons/app/tabs/playlist.png")), tr("Audio")); };

    switch (mode) {
        case UIMode::Full: {
            this->_leftTab->setVisible(true);

            removeFromTab(this->_rightTab, this->_audioManager);
            addPlaylistAudioManager(this->_leftTab);
            this->_audioManager->player()->setVisible(true);

            removeFromTab(this->_rightTab, this->_characterEditor);
            addCharacterEditor(this->_leftTab);

            this->_rightTab->addTab(this->_mlManager, QIcon(QStringLiteral(u":/icons/app/tabs/atom.png")), tr("Map Atoms"));
            this->_rightTab->addTab(this->_atomEditManager, QIcon(QStringLiteral(u":/icons/app/tabs/config.png")), tr("Atom Editor"));
        }
        break;

        case UIMode::Player: {
            this->_leftTab->setVisible(false);

            removeFromTab(this->_leftTab, this->_characterEditor);
            addCharacterEditor(this->_rightTab);

            removeFromTab(this->_leftTab, this->_audioManager);
            addPlaylistAudioManager(this->_rightTab);
            this->_audioManager->player()->setVisible(false);

            removeFromTab(this->_rightTab, this->_mlManager);
            removeFromTab(this->_rightTab, this->_atomEditManager);
        }
        break;

        default:
        break;
    }

    this->_currentAppUIMode = mode;
}

void MainWindow::_initAppComponents() {
    // init components
        this->_mapViewContainer = new QWidget(this);
        this->_mapViewContainer->setLayout(new OverlayingLayout);

        this->_mapTools = new MapTools(this);
        this->_mapActions = new MapActions(this);
        this->_mapView = new MapView(this);
        this->_minimap = new MiniMapView(this->_mapView, this);
        this->_mapHelpers = new MapHelpers(this->_minimap, this);

        this->_interactibleDescr = new MapViewInteractibleDescriptor(this);

        this->_mapViewContainer->layout()->addWidget(this->_mapView);
        this->_mapViewContainer->layout()->addWidget(this->_interactibleDescr);
        this->_mapViewContainer->layout()->setAlignment(this->_interactibleDescr, Qt::AlignTop);
        this->_mapViewContainer->layout()->addWidget(this->_minimap);

    this->_docShareManager = new DocShareListView(this);
    this->_chatWidget = new ChatWidget(this);
    this->_audioManager = new PlaylistAudioManager(this);
    this->_toys = new ToysTreeView(this);
    this->_mlManager = new MapLayoutManager(this->_mapView, this);
    this->_connectWidget = new ConnectWidget(this);
    this->_atomEditManager = new AtomEditionManager(this);
    this->_characterEditor = new CharacterEditor(this);
    this->_usersView = new StandardUsersListView(this);
    this->_playersView = new PlayersListView(this);

    //
    // EVENTS
    //

    QObject::connect(
        HintThread::hint(), &ViewMapHint::atomDescriptorUpdated,
        this->_interactibleDescr, &MapViewInteractibleDescriptor::updateFromAtom
    );

    QObject::connect(
        this->_mapHelpers, &QToolBar::actionTriggered,
        this->_mapView, &MapView::onHelperActionTriggered
    );

    // on default layer changed
    QObject::connect(
        this->_atomEditManager->layerSelector()->spinbox(), qOverload<int>(&QSpinBox::valueChanged),
        HintThread::hint(), &ViewMapHint::setDefaultLayer
    );

    // define default visibility
    QObject::connect(
        this->_atomEditManager->hiddenCheckbox(), &QCheckBox::stateChanged,
        HintThread::hint(), &ViewMapHint::setDefaultVisibility
    );

    // intercept preview request from editor
    QObject::connect(
        this->_atomEditManager->editor(), &AtomEditor::requiresPreview,
        HintThread::hint(), &ViewMapHint::handlePreviewRequest
    );

    // bind toolbar to mapview
    QObject::connect(
        this->_mapActions, &MapActions::actionRequested,
        this->_mapView, &MapView::onActionRequested
    );

    // focus on character
    QObject::connect(
        this->_playersView, &PlayersListView::requestingFocusOnCharacter,
        this, &MainWindow::_onCharacterFocusRequest
    );
    QObject::connect(
        this->_mapView, &MapView::requestingFocusOnCharacter,
        this, &MainWindow::_onCharacterFocusRequest
    );
}

void MainWindow::_initUIMenu() {
    // menu
    auto menuBar = new QMenuBar(this);
    menuBar->addMenu(this->_getFileMenu());
    auto mm = menuBar->addMenu(this->_getMapMenu());
    menuBar->addMenu(this->_getToolsMenu());
    menuBar->addMenu(this->_getDisplayMenu());
    menuBar->addMenu(this->_getHelpMenu());

    // on remote change detected...
    QObject::connect(
        HintThread::hint(), &MapHint::remoteChanged,
        [=](bool isRemote) {
            mm->setEnabled(!isRemote);
    });

    // set container
    this->setMenuWidget(menuBar);
}

void MainWindow::_initUIStatusBar() {
    this->_sb = new RPZStatusBar(this);
    this->setStatusBar(this->_sb);
}

void MainWindow::_onCharacterFocusRequest(const RPZCharacter::Id &characterIdToFocus) {
    this->_characterEditor->tryToSelectCharacter(characterIdToFocus);

    auto tab = (QTabWidget*)this->_characterEditor->parentWidget()->parentWidget();
    auto editorIndex = tab->indexOf(this->_characterEditor);

    tab->setCurrentIndex(editorIndex);
}

//////////////////
/// END UI init //
//////////////////

//////////////////////
/// Menu components //
//////////////////////

QMenu* MainWindow::_getMapMenu() {
    auto mapMenuItem = new QMenu(tr("Map"), this);

    // create map
    auto cmRPZmAction = RPZActions::createANewMap();
    QObject::connect(
        cmRPZmAction, &QAction::triggered,
        [=]() {
            // save beforehand if it have to
            HintThread::hint()->mayWantToSavePendingState(this);

            // dialog
            auto picked = QFileDialog::getSaveFileName(
                this,
                tr("Create a new map"),
                AppContext::getMapsFolderLocation(),
                tr("Game map (*%1)").arg(AppContext::RPZ_MAP_FILE_EXT)
            );
            if (picked.isNull()) return;

            // create !
            QMetaObject::invokeMethod(HintThread::hint(), "createNewRPZMapAs",
                Q_ARG(QString, picked)
            );
    });

    // load map
    auto lRPZmAction = RPZActions::loadAMap();
    QObject::connect(
        lRPZmAction, &QAction::triggered,
        [=]() {
            // save beforehand if it have to
            HintThread::hint()->mayWantToSavePendingState(this);

            // dialog
            auto picked = QFileDialog::getOpenFileName(
                this,
                tr("Load a new map"),
                AppContext::getMapsFolderLocation(),
                tr("Game map (*%1)").arg(AppContext::RPZ_MAP_FILE_EXT)
            );
            if (picked.isNull()) return;

            // load map
            QMetaObject::invokeMethod(HintThread::hint(), "loadRPZMap",
                Q_ARG(QString, picked)
            );
    });

    // save map
    auto sRPZmAction = RPZActions::saveTheMap();
    QObject::connect(
        sRPZmAction, &QAction::triggered,
        HintThread::hint(), &MapHint::saveRPZMap
    );

    // save as map
    auto saRPZmAction = RPZActions::saveTheMapAs();
    QObject::connect(
        saRPZmAction, &QAction::triggered,
        [=]() {
            auto picked = QFileDialog::getSaveFileName(
                this,
                tr("Save as..."),
                HintThread::hint()->mapFilePath(),
                tr("Game map (*%1)").arg(AppContext::RPZ_MAP_FILE_EXT)
            );

            if (!picked.isNull()) {
                QMetaObject::invokeMethod(HintThread::hint(), "saveRPZMapAs",
                    Q_ARG(QString, picked)
                );
            }
    });

    mapMenuItem->addAction(cmRPZmAction);
    mapMenuItem->addAction(lRPZmAction);
    mapMenuItem->addSeparator();
    mapMenuItem->addAction(sRPZmAction);
    mapMenuItem->addAction(saRPZmAction);

    return mapMenuItem;
}

QMenu* MainWindow::_getToolsMenu() {
    auto toolsMenuItem = new QMenu(tr("Tools"), this);

    // maintenance tool
    auto openMaintenanceToolAction = RPZActions::openMaintenanceTool();
    QObject::connect(
        openMaintenanceToolAction, &QAction::triggered,
        this->_updateIntegrator, &UpdaterUIIntegrator::openMaintenanceTool
    );

    // full log
    auto openLogAction = RPZActions::openFullLog();
    QObject::connect(
        openLogAction, &QAction::triggered,
        [&]() {
            AppContext::openFileInOS(
                AppContext::getLogFileLocation()
            );
    });

    // latest log
    auto openLatestLogAction = RPZActions::openLatestLog();
    QObject::connect(
        openLatestLogAction, &QAction::triggered,
        [&]() {
            AppContext::openFileInOS(
                AppContext::getLatestLogFileLocation()
            );
    });

    // data folder
    auto openDataFolderAction = RPZActions::openInternalDataFolder();
    QObject::connect(
        openDataFolderAction, &QAction::triggered,
        [&]() {
            AppContext::openFolderInOS(
                AppContext::getAppDataLocation()
            );
    });

    // layout
    toolsMenuItem->addAction(openMaintenanceToolAction);
    toolsMenuItem->addSeparator();
    toolsMenuItem->addAction(openDataFolderAction);
    toolsMenuItem->addSeparator();
    toolsMenuItem->addAction(openLogAction);
    toolsMenuItem->addAction(openLatestLogAction);

    return toolsMenuItem;
}

QMenu* MainWindow::_getHelpMenu() {
    auto helpMenuItem = new QMenu(tr("Help"), this);

    // for checking the upgrades available
    this->cfugAction = RPZActions::checkUpdates();
    QObject::connect(
        this->cfugAction, &QAction::triggered,
        this->_updateIntegrator, &UpdaterUIIntegrator ::requireUpdateCheckFromUser
    );

    // on updater state changed
    QObject::connect(
        this->_updateIntegrator, &UpdaterUIIntegrator::stateChanged,
        [&](const bool isSearching) {
            this->cfugAction->setEnabled(!isSearching);
            auto descr = isSearching ? tr("Searching for updates...") : tr("Check for updates...");
            this->cfugAction->setText(descr);
    });

    // patchnote
    auto patchnoteAction = RPZActions::patchnote();
    QObject::connect(
        patchnoteAction, &QAction::triggered,
        [&]() {
            auto destUrl = QUrl(APP_PATCHNOTE_URL);
            QDesktopServices::openUrl(destUrl);
    });

    // sentry
    auto testSentryReportAction = RPZActions::sentry();
    QObject::connect(
        testSentryReportAction, &QAction::triggered,
        [&]() {
            // must crash and trigger Sentry minidump report push
            abort();
    });

    helpMenuItem->addAction(patchnoteAction);
    helpMenuItem->addAction(testSentryReportAction);
    helpMenuItem->addSeparator();
    helpMenuItem->addAction(this->cfugAction);

    return helpMenuItem;
}

QMenu* MainWindow::_getFileMenu() {
    auto fileMenuItem = new QMenu(tr("File"), this);

    // quit
    auto quitAction = RPZActions::quit();
    QObject::connect(
        quitAction, &QAction::triggered,
        this, &MainWindow::close
    );
    fileMenuItem->addAction(quitAction);

    return fileMenuItem;
}

QMenu* MainWindow::_getDisplayMenu() {

    auto displayMenuItem = new QMenu(tr("Display"), this);

    // fullscreen mode button
    auto fullscreenAction = new QAction(tr("Fullscreen Mode"));
    fullscreenAction->setShortcut(QKeySequence("F11"));
    fullscreenAction->setShortcutVisibleInContextMenu(true);
    QObject::connect(
        fullscreenAction, &QAction::triggered,
        [=]() {
          if(!this->isFullScreen()) {
            this->showFullScreen();
          } else {
              this->showMaximized();
          }
        }
    );
    displayMenuItem->addAction(fullscreenAction);

    return displayMenuItem;
}

QToolBar* MainWindow::_getMenuToolbar() {
    auto menuToolbar = new QToolBar();
    menuToolbar->layout()->setMargin(0);
    menuToolbar->setIconSize(QSize(16, 16));
    menuToolbar->setMovable(true);
    menuToolbar->setFloatable(true);

    //
    menuToolbar->addSeparator();

    //
    auto hideAction = new QAction(QIcon(QStringLiteral(u":/icons/app/tools/menu.png")), tr("Hide menus and status bar"));
    hideAction->setCheckable(true);
    hideAction->setChecked(AppContext::settings()->mainWindowBarsShown());

        //
        menuToolbar->addAction(hideAction);
        QObject::connect(
            hideAction, &QAction::triggered,
            [=]() {
                // invert setting status
                AppContext::settings()->setMainWindowBarsShown(
                    !AppContext::settings()->mainWindowBarsShown()
                );

                //
                this->_triggerBarsVisibility();
            }
        );

    return menuToolbar;
}

//////////////////////////
/// END Menu components //
//////////////////////////

