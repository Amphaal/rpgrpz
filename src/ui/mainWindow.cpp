#include "MainWindow.h"


MainWindow::MainWindow() : _updateIntegrator(new UpdaterUIIntegrator(this)) {

    //init...
    this->_initUI();
    this->_initConnectivity();
    
    //initial show
    this->_loadWindowState();

    //start the update check
    this->_updateIntegrator->checkForAppUpdates();

    //load default map
    this->_mapView->hints()->loadDefaultState();
}

void MainWindow::_saveWindowState() {
    AppContext::settings()->beginGroup("mainWindow");
    AppContext::settings()->setValue("windowGeometry", this->saveGeometry());
    AppContext::settings()->setValue("windowState", this->saveState());
    AppContext::settings()->endGroup();
}

void MainWindow::_loadWindowState() {

    //default state to save
    if(!AppContext::settings()->childGroups().contains("mainWindow")) {
        this->showMaximized();
        this->_saveWindowState();
        return;
    }

    //load...
    AppContext::settings()->beginGroup("mainWindow");
    this->restoreGeometry(
        AppContext::settings()->value("windowGeometry").toByteArray()
    );
    this->restoreState(
        AppContext::settings()->value("windowState").toByteArray()
    );
    AppContext::settings()->endGroup();

    this->show();
}

void MainWindow::_trueShow() {
    this->showNormal();
    this->activateWindow();
    this->raise();
}

//handle clean close
void MainWindow::closeEvent(QCloseEvent *event) {

    //save map changes
    this->_mapView->hints()->mayWantToSavePendingState();

    //unbind network client from ui
    ClientBindable::unbindAll();

    //save window state
    this->_saveWindowState();
    
    //normal close
    QMainWindow::closeEvent(event);
}

void MainWindow::_initConnectivity() {
    
    //server behavior
    auto appArgs = AppContext::getOptionArgs(*QApplication::instance());
    if(appArgs.count() && appArgs.at(0) == "noServer") {    

        //do nothing
        this->_mustLaunchServer = false;
        qDebug() << "RPZServer : No server to start because the user said so.";
        this->_sb->updateServerStateLabel("Non", RPZStatusLabel::State::Finished);

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

        this->_rpzServer = new RPZServer(this);

        //tell the UI when the server is up
        QObject::connect(
            this->_rpzServer, &RPZServer::listening,
            [&]() {
                this->_sb->updateServerStateLabel("OK", RPZStatusLabel::State::Finished);
            }
        );

        //tell the UI when the server is down
        QObject::connect(
            this->_rpzServer, &RPZServer::error,
            [&]() {
                this->_sb->updateServerStateLabel("Erreur", RPZStatusLabel::State::Error);
            }
        );

        this->_rpzServer->run();

    }

}


//////////////
/// UI init //
//////////////

void MainWindow::_initUI() {
    
    this->setWindowTitle(AppContext::getWindowTitle());
    this->setWindowIcon(QIcon(":/icons/app/rpgrpz_32.png"));

    //central widget
    auto centralW = new RestoringSplitter("MainWindowSplitter");
    centralW->setContentsMargins(10, 5, 10, 10);
    this->setCentralWidget(centralW);

    //specific componements
    this->_initUIStatusBar();
    this->_initUIApp();
    this->_initUIMenu();

    //default focus
    this->_mapView->setFocus();
}   

void MainWindow::_initUIApp() {
    
    //init components
    this->_connectWidget = new ConnectWidget;
    this->_cw = new ChatWidget;
    this->_mapView = new MapView(this);
    this->_streamNotifier = new AudioStreamController;
    this->_streamController = new PlaylistController;
    this->_assetsManager = new AssetsManager;
    this->_mapTools = new MapTools;
    this->_mlManager = new MapLayoutManager;
    
    //audio...
    auto audio_content = new QWidget;
    audio_content->setLayout(new QVBoxLayout);
    audio_content->setContentsMargins(0, 0, 0, 0);
    audio_content->layout()->addWidget(this->_streamController);
    audio_content->layout()->addWidget(this->_streamNotifier);

    //assets
    auto assetTabs = new QTabWidget(this);
    assetTabs->addTab(this->_assetsManager, QIcon(":/icons/app/tabs/box.png"), "Boite à jouets");
    assetTabs->addTab(audio_content, QIcon(":/icons/app/tabs/playlist.png"), "Audio");

    //designer
    auto designer = new QWidget;
    designer->setLayout(new QVBoxLayout);
    designer->layout()->setMargin(0);
    designer->layout()->setSpacing(2);

        auto toolbar = new QWidget;
        auto toolbarLayout = new QHBoxLayout;
        toolbar->setLayout(toolbarLayout);
        toolbarLayout->setMargin(0);
        toolbarLayout->setSpacing(0);

        toolbarLayout->addWidget(this->_mapTools);
        toolbarLayout->addStretch(0);
        toolbarLayout->addWidget(this->_connectWidget);
        
    designer->layout()->addWidget(toolbar);
    designer->layout()->addWidget(this->_mapView);

    //right part tab
    auto eTabs = new QTabWidget(this);
    eTabs->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    eTabs->addTab(this->_mlManager, QIcon(":/icons/app/tabs/list.png"), "Atomes de la carte");
    eTabs->addTab(this->_cw, QIcon(":/icons/app/tabs/chat.png"), "Chat de la partie");

    //final
    auto centralWidget = (RestoringSplitter*)this->centralWidget();
    centralWidget->addWidget(assetTabs);
    centralWidget->addWidget(designer);
    centralWidget->addWidget(eTabs);
    centralWidget->setStretchFactor(0, 0);
    centralWidget->setStretchFactor(1, 1);
    centralWidget->setStretchFactor(2, 0);
    centralWidget->loadState();

    //
    // EVENTS
    //

    //bind RPZClient to widget once a connection starts
    QObject::connect(
        this->_connectWidget, &ConnectWidget::startingConnection, 
        ClientBindable::bindAll
    );

    //on map alteration, update treelist
    QObject::connect(
        this->_mapView->hints(), &ViewMapHint::alterationRequested,
        this->_mlManager->tree()->hints(), &TreeMapHint::handleAlterationRequest
    );

    //intercept alteration from layout manager
    QObject::connect(
        this->_mlManager->tree()->hints(), &TreeMapHint::elementsAlterationAsked,
        this->_mapView->hints(), &ViewMapHint::handleAlterationRequest
    );

    //on map selection change
    QObject::connect(
        this->_mapView->hints(), &ViewMapHint::selectionChanged,
        this->_mlManager->editor(), &AtomEditor::buildEditor
    );


    //on default layer changed
    QObject::connect(
        this->_mlManager->layerSelector()->spinbox(), qOverload<int>(&QSpinBox::valueChanged),
        this->_mapView->hints(), &ViewMapHint::setDefaultLayer
    );



    //intercept alteration from editor
    QObject::connect(
        this->_mlManager->editor(), &AtomEditor::requiresAtomAlteration,
        this->_mapView->hints(), &ViewMapHint::handleAlterationRequest
    );

    //unselect tools
    QObject::connect(
        this->_mapView, &MapView::unselectCurrentToolAsked,
        this->_mapTools, &MapTools::selectDefaultTool
    );

    //bind toolbar to mapview
    QObject::connect(
        this->_mapTools, &MapTools::toolSelectionChanged,
        this->_mapView, &MapView::changeToolFromAction
    );

    //on pen size change
    QObject::connect(
        this->_mapTools, &MapTools::penSizeChanged,
        this->_mapView->hints(), &ViewMapHint::setPenSize
    );

    //update status bar on map file update
    QObject::connect(
        this->_mapView->hints(), &ViewMapHint::mapFileStateChanged,
        this->_sb, &RPZStatusBar::updateMapFileLabel
    );
    this->_sb->updateMapFileLabel(
        this->_mapView->hints()->stateFilePath(),
        this->_mapView->hints()->isDirty()
    );

}

void MainWindow::_initUIMenu() {
    
    //menu
    auto menuBar = new QMenuBar;
    menuBar->addMenu(this->_getFileMenu());
    menuBar->addMenu(this->_getMapMenu());
    menuBar->addMenu(this->_getToolsMenu());
    menuBar->addMenu(this->_getHelpMenu());


    //set container
    this->setMenuWidget(menuBar);
}

void MainWindow::_initUIStatusBar() {

    this->_sb = new RPZStatusBar(this);
    this->setStatusBar(this->_sb);
}

//////////////////
/// END UI init //
//////////////////

//////////////////////
/// Menu components //
//////////////////////

QMenu* MainWindow::_getMapMenu() {

    auto mapMenuItem = new QMenu("Carte");

    //load map
    auto loadMap = RPZActions::loadMap();
    QObject::connect(
        loadMap, &QAction::triggered,
        this, &MainWindow::_loadMap
    );
    
    //save map
    auto saveMap = RPZActions::saveMap();
    QObject::connect(
        saveMap, &QAction::triggered,
        this->_mapView->hints(), &ViewMapHint::saveState
    );

    //save as map
    auto saveAsMap = RPZActions::saveAsMap();
    QObject::connect(
        saveAsMap, &QAction::triggered,
        this, &MainWindow::_saveAs
    );

    QList<QAction*> mapActions = { loadMap, saveMap, saveAsMap };

    //on remote change detected...
    QObject::connect(
        this->_mapView, &MapView::remoteChanged,
        [mapActions](bool isRemote) {
            for(auto action : mapActions) {
                action->setEnabled(!isRemote);
            }
        }
    );

    mapMenuItem->addAction(loadMap);
    mapMenuItem->addSeparator();
    mapMenuItem->addAction(saveMap);
    mapMenuItem->addAction(saveAsMap);

    return mapMenuItem;
}

void MainWindow::_saveAs() {
    auto picked = QFileDialog::getSaveFileName(this,
        "Enregistrer sous...",
        this->_mapView->hints()->stateFilePath(), 
        I18n::tr()->Popup_MapDescriptor()
    );

    if(!picked.isNull()) {
        this->_mapView->hints()->saveStateAs(picked);
    }
}
void MainWindow::_loadMap() {
    auto picked = QFileDialog::getOpenFileName(this, 
        "Ouvrir une carte", 
        AppContext::getMapsFolderLocation(), 
        I18n::tr()->Popup_MapDescriptor()
    );
    if(!picked.isNull()) {
        this->_mapView->hints()->loadState(picked);
    }
}

QMenu* MainWindow::_getToolsMenu() {

    auto toolsMenuItem = new QMenu(I18n::tr()->Menu_Tools());

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

    auto helpMenuItem = new QMenu(I18n::tr()->Menu_Help());

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
            const QString descr = isSearching ? I18n::tr()->SearchingForUpdates() : I18n::tr()->Menu_CheckForUpgrades();
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

    helpMenuItem->addAction(patchnoteAction);
    helpMenuItem->addSeparator();
    helpMenuItem->addAction(this->cfugAction);

    return helpMenuItem;
};

QMenu* MainWindow::_getFileMenu() {

    auto fileMenuItem = new QMenu(I18n::tr()->Menu_File());

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

