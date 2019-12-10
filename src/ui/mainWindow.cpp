#include "mainWindow.h"

MainWindow::MainWindow() : _updateIntegrator(new UpdaterUIIntegrator(this)) {

    //start hint thread
    HintThread::init();

    //bind AlterationHandler / ProgressTracker / ConnectivityObserverOchestrator to UI Thread
    AlterationHandler::get();
    ProgressTracker::get();
    ConnectivityObserverSynchronizer::get();

    //
    AppContext::definePPcm(this);

    //init databases
    AssetsDatabase::get();
    CharactersDatabase::get();

    //init...
    this->_initUI();
    this->_initConnectivity();

    //initial show
    this->_loadWindowState();

    this->_triggerBarsVisibility();

    //load default map
    QMetaObject::invokeMethod(HintThread::hint(), "loadDefaultRPZMap");
    
    //start the update check
    this->_updateIntegrator->checkForAppUpdates();

}

MainWindow::~MainWindow() {
    if(this->_rpzServer) this->_rpzServer->thread()->quit();

    //unbind network client from ui
    ConnectivityObserver::disconnectClient();
    
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

void MainWindow::connectionClosed(bool hasInitialMapLoaded) {
    this->_setupAppUI(_defaultAppUIMode);
}

void MainWindow::keyPressEvent(QKeyEvent * event) {
    
    switch(event->key()) {

        case Qt::Key_Alt: {
            this->_showBars = !this->_showBars;
            this->_triggerBarsVisibility();
        }
        break;

        default:
        break;

    }
    
}

void MainWindow::_triggerBarsVisibility() {
    this->menuBar()->setVisible(this->_showBars);
    this->statusBar()->setVisible(this->_showBars);
}

void MainWindow::_barVisibilityToolTip() {
    
    //fix tooltip pos
    auto pos = this->geometry().bottomRight();
    pos.setY(pos.y() - 37);

    //display it
    QToolTip::showText(
        pos, 
        tr("Press ALT key to display the menu !"),
        this,
        this->geometry(),
        3000
    );

}

void MainWindow::_saveWindowState() {
    AppContext::settings()->beginGroup(QStringLiteral(u"mainWindow"));
    AppContext::settings()->setValue(QStringLiteral(u"windowGeometry"), this->saveGeometry());
    AppContext::settings()->setValue(QStringLiteral(u"windowState"), this->saveState());
    AppContext::settings()->endGroup();
}

void MainWindow::_loadWindowState() {

    //default state to save
    if(!AppContext::settings()->childGroups().contains(QStringLiteral(u"mainWindow"))) {
        this->showMaximized();
        this->_saveWindowState();
        return;
    }

    //load...
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

//handle clean close
void MainWindow::closeEvent(QCloseEvent *event) {

    //may save map changes
    MapHint::mayWantToSavePendingState(this, HintThread::hint());

    //save window state
    this->_saveWindowState();
    
    //normal close
    QMainWindow::closeEvent(event);
}

void MainWindow::_initConnectivity() {
    
    //check if we must use server capabilities
    auto appArgs = AppContext::getOptionArgs(*QApplication::instance());
    if(appArgs.contains(QStringLiteral(u"noServer"))) {    
        this->_mustLaunchServer = false;
        qDebug() << "RPZServer : No server to start because the user said so.";
        this->_sb->updateServerStateLabel(tr("No"), RPZStatusLabel::State::Finished);
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

    this->_ipHelper->init();

    ////////////////////
    /// RPZServer ! //
    ////////////////////

    //si serveur est local et lié à l'app
    if(this->_mustLaunchServer) {    

        this->_rpzServer = new RPZServer;

        this->_sb->bindServerIndicators();

        //tell the UI when the server is up
        QObject::connect(
            this->_rpzServer, &RPZServer::listening,
            [&]() {
                QMetaObject::invokeMethod(this->_sb, "updateServerStateLabel",
                    Q_ARG(QString, "OK"), 
                    Q_ARG(RPZStatusLabel::State, RPZStatusLabel::State::Finished)
                );
            }
        );

        //tell the UI when the server is down
        QObject::connect(
            this->_rpzServer, &RPZServer::error,
            [&]() {

                QMetaObject::invokeMethod(this->_sb, "updateServerStateLabel",
                    Q_ARG(QString, tr("Error")), 
                    Q_ARG(RPZStatusLabel::State, RPZStatusLabel::State::Error)
                );

                this->_rpzServer = nullptr;

            }
        );

        //create a separate thread to run the server into
        auto serverThread = new QThread;
        serverThread->setObjectName(QStringLiteral(u"RPZServer Thread"));
        this->_rpzServer->moveToThread(serverThread);
        
        //events...
        QObject::connect(
            serverThread, &QThread::started, 
            this->_rpzServer, &RPZServer::run
        );

        QObject::connect(
            this->_rpzServer, &RPZServer::stopped,  
            serverThread, &QThread::quit
        );

        QObject::connect(
            serverThread, &QThread::finished, 
            this->_rpzServer, &QObject::deleteLater
        );

        QObject::connect(
            serverThread, &QThread::finished, 
            serverThread, &QObject::deleteLater
        );

        //start
        serverThread->start();

    }

}


//////////////
/// UI init //
//////////////

void MainWindow::_initUI() {
    
    this->setWindowTitle(AppContext::getWindowTitle());
    this->setWindowIcon(QIcon(QStringLiteral(u":/icons/app/app_32.png")));

    //central widget
    auto centralW = new RestoringSplitter("MainWindowSplitter");
    centralW->setContentsMargins(5, 5, 5, 5);
    this->setCentralWidget(centralW);

    //specific componements
    this->_initUIStatusBar();
    this->_initAppComponents();
    this->_initAppUnmovableUI();
    this->_setupAppUI(_defaultAppUIMode);
    this->_initUIMenu();

    //visibility
    this->_triggerBarsVisibility();

    //default focus
    this->_mapView->setFocus();
}   

void MainWindow::_initAppUnmovableUI() {
    
    //left tabs
    this->_leftTab = new QTabWidget(this);
    this->_leftTab->addTab(this->_toys, QIcon(QStringLiteral(u":/icons/app/tabs/box.png")), tr("Toy box"));

    //right tabs
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
    
    this->_rightTab->addTab(chatLogWidget, QIcon(QStringLiteral(u":/icons/app/tabs/chat.png")), tr("Game Hub"));

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
            
        layout->addWidget(toolbar, 0, Qt::AlignTop);
        layout->addWidget(this->_mapViewContainer, 1);

        return designer;
    }();

    //final
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
    
    if(mode == this->_currentAppUIMode) return;

    auto removeFromTab = [](QTabWidget* tab, QWidget* toRemove) {
        auto index = tab->indexOf(toRemove);
        if(index == -1) return;
        tab->removeTab(index);
    };

    auto addCharacterEditor = [=](QTabWidget* tab) {tab->addTab(this->_characterEditor, QIcon(QStringLiteral(u":/icons/app/tabs/scroll.png")), tr("Sheets"));};
    auto addAudioManager = [=](QTabWidget* tab) { tab->addTab(this->_audioManager, QIcon(QStringLiteral(u":/icons/app/tabs/playlist.png")), tr("Audio")); };

    switch(mode) {
        
        case UIMode::Full: {
            this->_leftTab->setVisible(true);

            removeFromTab(this->_rightTab, this->_audioManager);
            addAudioManager(this->_leftTab);
            this->_audioManager->player()->setVisible(true);
            
            removeFromTab(this->_rightTab, this->_characterEditor);
            addCharacterEditor(this->_leftTab);

            this->_rightTab->insertTab(0, this->_mlManager, QIcon(QStringLiteral(u":/icons/app/tabs/atom.png")), tr("Map Atoms"));
            this->_rightTab->insertTab(0, this->_atomEditManager, QIcon(QStringLiteral(u":/icons/app/tabs/config.png")), tr("Atom Editor"));

        }
        break;

        case UIMode::Player: {

            this->_leftTab->setVisible(false);

            removeFromTab(this->_leftTab, this->_characterEditor);
            addCharacterEditor(this->_rightTab);

            removeFromTab(this->_leftTab, this->_audioManager);
            addAudioManager(this->_rightTab);
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
    
    //init components
        this->_mapViewContainer = new QWidget(this);
        this->_mapViewContainer->setLayout(new OverlayingLayout);

        this->_mapView = new MapView(this);
        this->_minimap = new MiniMapView(this->_mapView, this);
        this->_interactibleDescr = new MapViewInteractibleDescriptor(this);

        this->_mapViewContainer->layout()->addWidget(this->_mapView);
        this->_mapViewContainer->layout()->addWidget(this->_interactibleDescr);
        this->_mapViewContainer->layout()->setAlignment(this->_interactibleDescr, Qt::AlignTop);
        this->_mapViewContainer->layout()->addWidget(this->_minimap);
        
        this->_mapHelpers = new MapHelpers(this->_minimap, this);
        this->_mapActions = new MapActions(this);
    
    this->_chatWidget = new ChatWidget(this);
    this->_audioManager = new AudioManager(this);
    this->_toys = new ToysTreeView(this);
    this->_mapTools = new MapTools(this);
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

    //on default layer changed
    QObject::connect(
        this->_atomEditManager->layerSelector()->spinbox(), qOverload<int>(&QSpinBox::valueChanged),
        HintThread::hint(), &ViewMapHint::setDefaultLayer
    );

    //define default visibility
    QObject::connect(
        this->_atomEditManager->hiddenCheckbox(), &QCheckBox::stateChanged,
        HintThread::hint(), &ViewMapHint::setDefaultVisibility
    );

    //intercept preview request from editor
    QObject::connect(
        this->_atomEditManager->editor(), &AtomEditor::requiresPreview,
        HintThread::hint(), &ViewMapHint::handlePreviewRequest
    );

    //bind toolbar to mapview
    QObject::connect(
        this->_mapActions, &MapActions::actionRequested,
        this->_mapView, &MapView::onActionRequested
    );

    //update status bar on map file update
    QObject::connect(
        HintThread::hint(), &MapHint::mapStateChanged,
        this->_sb, &RPZStatusBar::updateMapFileLabel
    );

    //focus on character
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
    
    //menu
    auto menuBar = new QMenuBar(this);
    menuBar->addMenu(this->_getFileMenu());
    auto mm = menuBar->addMenu(this->_getMapMenu());
    menuBar->addMenu(this->_getToolsMenu());
    menuBar->addMenu(this->_getHelpMenu());

    //on remote change detected...
    QObject::connect(
        this->_mapView, &MapView::remoteChanged,
        [=](bool isRemote) {
            mm->setEnabled(!isRemote);
        }
    );

    //set container
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

    //create map
    auto cmRPZmAction = RPZActions::createANewMap();
    QObject::connect(
        cmRPZmAction, &QAction::triggered,
        [=]() {
            
            //save beforehand if it have to
            MapHint::mayWantToSavePendingState(this, HintThread::hint());

            //dialog
            auto picked = QFileDialog::getSaveFileName(
                this, 
                tr("Create a new map"), 
                AppContext::getMapsFolderLocation(), 
                tr("Game map (*%1)").arg(AppContext::RPZ_MAP_FILE_EXT)
            );
            if(picked.isNull()) return;

            //create !
            QMetaObject::invokeMethod(HintThread::hint(), "createNewRPZMapAs", 
                Q_ARG(QString, picked)
            );
            
        }
    );

    //load map
    auto lRPZmAction = RPZActions::loadAMap();
    QObject::connect(
        lRPZmAction, &QAction::triggered,
        [=]() {
            
            //save beforehand if it have to
            MapHint::mayWantToSavePendingState(this, HintThread::hint());
            
            //dialog
            auto picked = QFileDialog::getOpenFileName(
                this, 
                tr("Load a new map"), 
                AppContext::getMapsFolderLocation(), 
                tr("Game map (*%1)").arg(AppContext::RPZ_MAP_FILE_EXT)
            );
            if(picked.isNull()) return;

            //load map
            QMetaObject::invokeMethod(HintThread::hint(), "loadRPZMap", 
                Q_ARG(QString, picked)
            );
            
        }
    );
    
    //save map
    auto sRPZmAction = RPZActions::saveTheMap();
    QObject::connect(
        sRPZmAction, &QAction::triggered,
        HintThread::hint(), &MapHint::saveRPZMap
    );

    //save as map
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

            if(!picked.isNull()) {
                QMetaObject::invokeMethod(HintThread::hint(), "saveRPZMapAs", 
                    Q_ARG(QString, picked)
                );
            }
        }
    );

    mapMenuItem->addAction(cmRPZmAction);
    mapMenuItem->addAction(lRPZmAction);
    mapMenuItem->addSeparator();
    mapMenuItem->addAction(sRPZmAction);
    mapMenuItem->addAction(saRPZmAction);

    return mapMenuItem;
}

QMenu* MainWindow::_getToolsMenu() {

    auto toolsMenuItem = new QMenu(tr("Tools"), this);

    //maintenance tool
    auto openMaintenanceToolAction = RPZActions::openMaintenanceTool();
    QObject::connect(
        openMaintenanceToolAction, &QAction::triggered,
        this->_updateIntegrator, &UpdaterUIIntegrator::openMaintenanceTool
    );

    //full log
    auto openLogAction = RPZActions::openFullLog();
    QObject::connect(
        openLogAction, &QAction::triggered,
        [&]() {
            AppContext::openFileInOS(
                AppContext::getLogFileLocation()
            );
        }
    );

    //latest log
    auto openLatestLogAction = RPZActions::openLatestLog();
    QObject::connect(
        openLatestLogAction, &QAction::triggered,
        [&]() {
            AppContext::openFileInOS(
                AppContext::getLatestLogFileLocation()
            );
        }
    );

    //data folder
    auto openDataFolderAction = RPZActions::openInternalDataFolder();
    QObject::connect(
        openDataFolderAction, &QAction::triggered,
        [&]() {
            AppContext::openFolderInOS(
                AppContext::getAppDataLocation()
            );
        }
    );

    //layout
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

    //for checking the upgrades available
    this->cfugAction = RPZActions::checkUpdates();
    QObject::connect(
        this->cfugAction, &QAction::triggered,
        this->_updateIntegrator, &UpdaterUIIntegrator ::requireUpdateCheckFromUser
    );

    //on updater state changed
    QObject::connect(
        this->_updateIntegrator, &UpdaterUIIntegrator::stateChanged,
        [&](const bool isSearching) {
            this->cfugAction->setEnabled(!isSearching);
            auto descr = isSearching ? tr("Searching for updates...") : tr("Check for updates...");
            this->cfugAction->setText(descr);
        }
    );

    //patchnote
    auto patchnoteAction = RPZActions::patchnote();
    QObject::connect(
        patchnoteAction, &QAction::triggered,
        [&]() {
            auto destUrl = QUrl(APP_PATCHNOTE_URL);
            QDesktopServices::openUrl(destUrl);
        }
    );

    //sentry
    auto testSentryReportAction = RPZActions::sentry();
    QObject::connect(
        testSentryReportAction, &QAction::triggered,
        [&]() {
            QList<int> q;
            q.at(0);
        }
    );

    helpMenuItem->addAction(patchnoteAction);
    helpMenuItem->addAction(testSentryReportAction);
    helpMenuItem->addSeparator();
    helpMenuItem->addAction(this->cfugAction);

    return helpMenuItem;
};

QMenu* MainWindow::_getFileMenu() {

    auto fileMenuItem = new QMenu(tr("File"), this);

    //quit
    auto quitAction = RPZActions::quit();
    QObject::connect(
        quitAction, &QAction::triggered,
        this, &MainWindow::close
    );

    fileMenuItem->addAction(quitAction);

    return fileMenuItem;
};

//////////////////////////
/// END Menu components //
//////////////////////////

