#include "MainWindow.h"


MainWindow::MainWindow() : _updateIntegrator(new UpdaterUIIntegrator(this)) {

    //init...
    this->_initUI();
    this->_initConnectivity();
    
    //initial show
    this->resize(800, 600);
    this->showMaximized();
}

void MainWindow::_trueShow() {
    this->showNormal();
    this->activateWindow();
    this->raise();
}

//handle clean close
void MainWindow::closeEvent(QCloseEvent *event) {

    ClientBindable::unbindAll();

    event->accept();
}


void MainWindow::_initConnectivity() {
    
    //server behavior
    const auto appArgs = QApplication::instance()->arguments();
    if(appArgs.size() > 1 && appArgs.at(1) == "noServer") {    

        //do nothing
        this->_mustLaunchServer = false;
        qDebug() << "RPZServer : No server to start because the user said so.";
        this->_sb->updateServerStateLabel("Non");

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
                this->_sb->updateServerStateLabel("OK");
            }
        );

        //tell the UI when the server is down
        QObject::connect(
            this->_rpzServer, &RPZServer::error,
            [&]() {
                this->_sb->updateServerStateLabel("Erreur");
            }
        );

        //tell the UI when music is streaming
        QObject::connect(
            this->_rpzServer->audioServer(), &AudioServer::kbpsSent,
            this->_sb, &RPZStatusBar::updateSentAudioKbps
        );

        this->_rpzServer->run();

    }

}


//////////////
/// UI init //
//////////////

void MainWindow::_initUI() {
    
    //values specific to this
    const std::string stdTitle = IS_DEBUG_APP ? (std::string)"DEBUG - " + APP_FULL_DENOM : APP_FULL_DENOM;
    this->setWindowTitle(QString(stdTitle.c_str()));

    this->setWindowIcon(QIcon(":/icons/app/rpgrpz_32.png"));

    //central widget
    auto centralW = new RestoringSplitter("MainWindowSplitter");
    centralW->setContentsMargins(10, 5, 10, 10);
    this->setCentralWidget(centralW);

    //specific componements
    this->_initUIMenu();
    this->_initUIStatusBar();
    this->_initUIApp();

    //default focus
    this->_mapView->setFocus();
}   

void MainWindow::_initUIApp() {
    
    //init components
    this->_connectWidget = new ConnectWidget(this);
    this->_cw = new ChatWidget(this);
    this->_mapView = new MapView(this);
    this->_streamNotifier = new AudioStreamController(this);
    this->_streamController = new PlaylistController(this);
    this->_assetsManager = new AssetsManager(this);
    this->_mapTools = new MapTools(this);
    this->_mlManager = new MapLayoutManager(this);
    
    //assets
    auto assetTabs = new QTabWidget(this);
    assetTabs->addTab(this->_assetsManager, QIcon(":/icons/app/tabs/box.png"), "Boite à jouets");
    assetTabs->addTab(this->_mlManager, QIcon(":/icons/app/tabs/list.png"), "Elements de la carte");

    //designer
    auto designer = new QWidget;
    designer->setLayout(new QVBoxLayout);
    designer->layout()->setMargin(0);
    designer->layout()->setSpacing(2);
    designer->layout()->addWidget(this->_mapTools);
    designer->layout()->addWidget(this->_mapView);

    //Chat...
    auto audio = new QWidget;
    audio->setLayout(new QVBoxLayout);
    audio->setContentsMargins(0, 0, 0, 0);
    audio->layout()->addWidget(this->_streamController);
    audio->layout()->addWidget(this->_streamNotifier);
    
    auto eTabs = new QTabWidget(this);
    eTabs->addTab(this->_cw, QIcon(":/icons/app/tabs/chat.png"), "Chat de la partie");
    eTabs->addTab(audio, QIcon(":/icons/app/tabs/playlist.png"), "Audio");

    auto right = new QWidget;
    right->setLayout(new QVBoxLayout);
    right->layout()->setMargin(0);
    right->layout()->addWidget(this->_connectWidget);
    right->layout()->addWidget(eTabs);
    
    //final
    auto centralWidget = (RestoringSplitter*)this->centralWidget();
    centralWidget->addWidget(assetTabs);
    centralWidget->addWidget(designer);
    centralWidget->addWidget(right);
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

    //move to map content tab when selection changed inside the map
    QObject::connect(
        this->_mapView->scene(), &QGraphicsScene::selectionChanged,
        [assetTabs]() {
            assetTabs->setCurrentIndex(1);
        }
    );

    //on map alteration, update treelist
    QObject::connect(
        this->_mapView->hints(), &MapHintViewBinder::assetsAlteredForLocal,
        this->_mlManager, &MapLayoutManager::alterTreeElements
    );

    //intercept alteration from layout manager
    QObject::connect(
        this->_mlManager, &MapLayoutManager::elementsAlterationAsked,
        this->_mapView->hints(), &MapHintViewBinder::alterSceneFromIds
    );

    //unselect tools
    QObject::connect(
        this->_mapView, &MapView::unselectCurrentToolAsked,
        this->_mapTools, &MapTools::unselectAllTools
    );

    //bind toolbar to mapview
    QObject::connect(
        this->_mapTools, &MapTools::toolSelectionChanged,
        this->_mapView, &MapView::changeToolFromAction
    );

    //on pen size change
    QObject::connect(
        this->_mapTools, &MapTools::penSizeChanged,
        this->_mapView, &MapView::changePenSize
    );

}

void MainWindow::_initUIMenu() {
    auto menuBar = new QMenuBar;
    menuBar->addMenu(this->_getFileMenu());
    menuBar->addMenu(this->_getToolsMenu());
    menuBar->addMenu(this->_getHelpMenu());
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


QMenu* MainWindow::_getToolsMenu() {

    auto toolsMenuItem = new QMenu(I18n::tr()->Menu_Tools().c_str());

    //full log
    auto openLogAction = new QAction(I18n::tr()->Menu_OpenLog().c_str(), toolsMenuItem);
    QObject::connect(
        openLogAction, &QAction::triggered,
        [&]() {
            openFileInOS(getLogFileLocation());
        }
    );

    //latest log
    auto openLatestLogAction = new QAction(I18n::tr()->Menu_OpenLatestLog().c_str(), toolsMenuItem);
    QObject::connect(
        openLatestLogAction, &QAction::triggered,
        [&]() {
            openFileInOS(getLatestLogFileLocation());
        }
    );

    //data folder
    const auto df = getAppDataLocation();
    auto openDataFolderAction = new QAction(I18n::tr()->Menu_OpenDataFolder(df).c_str(), toolsMenuItem);
    QObject::connect(
        openDataFolderAction, &QAction::triggered,
        [&, df]() {
            openFolderInOS(df);
        }
    );

    toolsMenuItem->addAction(openDataFolderAction);
    toolsMenuItem->addSeparator();
    toolsMenuItem->addAction(openLogAction);
    toolsMenuItem->addAction(openLatestLogAction);

    return toolsMenuItem;
}

QMenu* MainWindow::_getHelpMenu() {

    auto helpMenuItem = new QMenu(I18n::tr()->Menu_Help().c_str());

    //for checking the upgrades available
    this->cfugAction = new QAction(I18n::tr()->Menu_CheckForUpgrades().c_str(), helpMenuItem);
    QObject::connect(
        this->cfugAction, &QAction::triggered,
        this->_updateIntegrator, &UpdaterUIIntegrator ::requireUpdateCheckFromUser
    );

    //on updater state changed
    QObject::connect(
        this->_updateIntegrator, &UpdaterUIIntegrator::stateChanged,
        [&](const bool isSearching) {
            this->cfugAction->setEnabled(!isSearching);
            const std::string descr = isSearching ? I18n::tr()->SearchingForUpdates() : I18n::tr()->Menu_CheckForUpgrades();
            this->cfugAction->setText(descr.c_str());
        }
    );

    //patchnote
    auto patchnoteAction = new QAction(I18n::tr()->Menu_Patchnotes(APP_FULL_DENOM).c_str(), helpMenuItem);
    QObject::connect(
        patchnoteAction, &QAction::triggered,
        [&]() {
            QDesktopServices::openUrl(QUrl(APP_PATCHNOTE_URL));
        }
    );

    helpMenuItem->addAction(patchnoteAction);
    helpMenuItem->addSeparator();
    helpMenuItem->addAction(this->cfugAction);

    return helpMenuItem;
};

QMenu* MainWindow::_getFileMenu() {

    auto fileMenuItem = new QMenu(I18n::tr()->Menu_File().c_str());

    //quit
    auto quitAction = new QAction(I18n::tr()->Menu_Quit().c_str(), fileMenuItem);
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

