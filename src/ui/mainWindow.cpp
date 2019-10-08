#include "mainWindow.h"

MainWindow::~MainWindow() {
    if(this->_rpzServer) {
        this->_rpzServer->thread()->quit();
        this->_rpzServer->thread()->wait();
    }

    //unbind network client from ui
    ClientBindable::unbindAll();
}

MainWindow::MainWindow() : _updateIntegrator(new UpdaterUIIntegrator(this)) {

    //bind AlterationHandler / ProgressTracker / ClientBindableMain to UI Thread
    AlterationHandler::get();
    ProgressTracker::get();
    ClientBindableMain::get();

    //init...
    this->_initUI();
    this->_initConnectivity();

    //initial show
    this->_loadWindowState();

    //load default map
    QMetaObject::invokeMethod(this->_mapView->hints(), "loadDefaultRPZMap");

    //start the update check
    this->_updateIntegrator->checkForAppUpdates();

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
        "Appuyez sur la touche ALT pour afficher le menu !",
        this,
        this->geometry(),
        3000
    );

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
    QTimer::singleShot(1000, [=]() {
        this->_barVisibilityToolTip();
    });
}

void MainWindow::_trueShow() {
    this->showNormal();
    this->activateWindow();
    this->raise();
}

//handle clean close
void MainWindow::closeEvent(QCloseEvent *event) {

    //may save map changes
    MapHint::mayWantToSavePendingState(this, this->_mapView->hints());

    //save window state
    this->_saveWindowState();
    
    //normal close
    QMainWindow::closeEvent(event);
}

void MainWindow::_initConnectivity() {
    
    //check if we must use server capabilities
    auto appArgs = AppContext::getOptionArgs(*QApplication::instance());
    if(appArgs.contains("noServer")) {    
        this->_mustLaunchServer = false;
        qDebug() << "RPZServer : No server to start because the user said so.";
        this->_sb->updateServerStateLabel("Non", SLState::SL_Finished);
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
                    Q_ARG(SLState, SLState::SL_Finished)
                );
            }
        );

        //tell the UI when the server is down
        QObject::connect(
            this->_rpzServer, &RPZServer::error,
            [&]() {

                QMetaObject::invokeMethod(this->_sb, "updateServerStateLabel",
                    Q_ARG(QString, "Erreur"), 
                    Q_ARG(SLState, SLState::SL_Error)
                );

                this->_rpzServer = nullptr;

            }
        );

        //create a separate thread to run the server into
        auto serverThread = new QThread;
        serverThread->setObjectName("RPZServer Thread");
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
            this->_rpzServer, &RPZServer::stopped,  
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
    this->setWindowIcon(QIcon(":/icons/app/rpgrpz_32.png"));

    //central widget
    auto centralW = new RestoringSplitter("MainWindowSplitter");
    centralW->setContentsMargins(5, 5, 5, 5);
    this->setCentralWidget(centralW);

    //specific componements
    this->_initUIStatusBar();
    this->_initUIApp();
    this->_initUIMenu();

    //visibility
    this->_triggerBarsVisibility();

    //default focus
    this->_mapView->setFocus();
}   

void MainWindow::_initUIApp() {
    
    //init components
    this->_cw = new ChatWidget(this);
    this->_mapView = new MapView(this);
    this->_audioManager = new AudioManager(this);
    this->_assetsManager = new AssetsManager(this);
    this->_mapTools = new MapTools(this);
    this->_mlManager = new MapLayoutManager(this->_mapView->hints(), this);
    this->_connectWidget = new ConnectWidget(this->_mapView->hints(), this);
    this->_atomEditManager = new AtomEditionManager(this->_mapView->hints(), this);
    this->_characterEditor = new CharacterEditor(this);
    this->_usersView = new PlayersListView(this);
    
    //left tabs
    auto lTab = new QTabWidget(this);
    lTab->addTab(this->_assetsManager, QIcon(":/icons/app/tabs/box.png"), "Boite à jouets");
    lTab->addTab(this->_audioManager, QIcon(":/icons/app/tabs/playlist.png"), "Audio");
    lTab->addTab(this->_characterEditor, QIcon(":/icons/app/tabs/scroll.png"), "Fiches");

    //right tabs
    auto rTab = new QTabWidget(this);
    rTab->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    rTab->addTab(this->_mlManager, QIcon(":/icons/app/tabs/list.png"), "Atomes de la carte");
    rTab->addTab(this->_atomEditManager, QIcon(":/icons/app/tabs/config.png"), "Editeur d'atome");
    
    auto CCw = new QWidget(this);
    auto CCwLayout = new QHBoxLayout(this);
    CCwLayout->setDirection(QBoxLayout::RightToLeft);
    CCw->setLayout(CCwLayout);
    
    auto logLayout = new QVBoxLayout(this);
    logLayout->addWidget(this->_connectWidget); 
    logLayout->addWidget(this->_cw);

    CCwLayout->addLayout(logLayout, 1);
    CCwLayout->addWidget(this->_usersView);

    rTab->addTab(CCw, QIcon(":/icons/app/tabs/chat.png"), "Connexion / Chat");

    //designer
    auto designer = new QWidget(this);
    designer->setLayout(new QVBoxLayout(this));
    designer->layout()->setMargin(0);
    designer->layout()->setSpacing(2);

        auto toolbar = new QWidget(this);
        auto toolbarLayout = new QHBoxLayout(this);
        toolbar->setLayout(toolbarLayout);
        toolbarLayout->setMargin(0);
        toolbarLayout->setSpacing(0);

        toolbarLayout->addStretch(0);
        toolbarLayout->addWidget(this->_mapTools);
        
    designer->layout()->addWidget(toolbar);
    designer->layout()->addWidget(this->_mapView);

    //final
    auto centralWidget = (RestoringSplitter*)this->centralWidget();
    centralWidget->addWidget(lTab);
    centralWidget->addWidget(designer);
    centralWidget->addWidget(rTab);
    centralWidget->setStretchFactor(0, 0);
    centralWidget->setStretchFactor(1, 1);
    centralWidget->setStretchFactor(2, 0);
    centralWidget->restore();

    //
    // EVENTS
    //

    //bind RPZClient to widget once a connection starts
    QObject::connect(
        this->_connectWidget, &ConnectWidget::startingConnection, 
        ClientBindable::bindAll
    );

    //on default layer changed
    QObject::connect(
        this->_mlManager->layerSelector()->spinbox(), qOverload<int>(&QSpinBox::valueChanged),
        this->_mapView->hints(), &ViewMapHint::setDefaultLayer
    );

    //intercept preview request from editor
    QObject::connect(
        this->_atomEditManager->editor(), &AtomEditor::requiresPreview,
        this->_mapView->hints(), &ViewMapHint::handlePreviewRequest
    );

    //bind toolbar to mapview
    QObject::connect(
        this->_mapTools, &MapTools::actionRequested,
        this->_mapView, &MapView::onActionRequested
    );

    //update status bar on map file update
    QObject::connect(
        this->_mapView->hints(), &MapHint::mapStateChanged,
        this->_sb, &RPZStatusBar::updateMapFileLabel
    );
}

void MainWindow::_initUIMenu() {
    
    //menu
    auto menuBar = new QMenuBar(this);
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

    auto mapMenuItem = new QMenu("Carte", this);

    //create map
    auto cmRPZmAction = RPZActions::createANewMap();
    QObject::connect(
        cmRPZmAction, &QAction::triggered,
        [=]() {
            
            //save beforehand if it have to
            MapHint::mayWantToSavePendingState(this, this->_mapView->hints());

            //dialog
            auto picked = QFileDialog::getSaveFileName(
                this, 
                "Créer une nouvelle carte", 
                AppContext::getMapsFolderLocation(), 
                I18n::tr()->Popup_MapDescriptor()
            );
            if(picked.isNull()) return;

            //create !
            QMetaObject::invokeMethod(this->_mapView->hints(), "createNewRPZMapAs", 
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
            MapHint::mayWantToSavePendingState(this, this->_mapView->hints());
            
            //dialog
            auto picked = QFileDialog::getOpenFileName(
                this, 
                "Ouvrir une carte", 
                AppContext::getMapsFolderLocation(), 
                I18n::tr()->Popup_MapDescriptor()
            );
            if(picked.isNull()) return;

            //load map
            QMetaObject::invokeMethod(this->_mapView->hints(), "loadRPZMap", 
                Q_ARG(QString, picked)
            );
            
        }
    );
    
    //save map
    auto sRPZmAction = RPZActions::saveTheMap();
    QObject::connect(
        sRPZmAction, &QAction::triggered,
        this->_mapView->hints(), &MapHint::saveRPZMap
    );

    //save as map
    auto saRPZmAction = RPZActions::saveTheMapAs();
    QObject::connect(
        saRPZmAction, &QAction::triggered,
        [=]() {
            auto picked = QFileDialog::getSaveFileName(
                this,
                "Enregistrer sous...",
                this->_mapView->hints()->RPZMapFilePath(), 
                I18n::tr()->Popup_MapDescriptor()
            );

            if(!picked.isNull()) {
                QMetaObject::invokeMethod(this->_mapView->hints(), "saveRPZMapAs", Q_ARG(QString, picked));
            }
        }
    );

    QList<QAction*> mapActions = { lRPZmAction, sRPZmAction, saRPZmAction };

    //on remote change detected...
    QObject::connect(
        this->_mapView, &MapView::remoteChanged,
        [mapActions](bool isRemote) {
            for(auto action : mapActions) {
                action->setEnabled(!isRemote);
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

    auto toolsMenuItem = new QMenu(I18n::tr()->Menu_Tools(), this);

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

    auto helpMenuItem = new QMenu(I18n::tr()->Menu_Help(), this);

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

    auto fileMenuItem = new QMenu(I18n::tr()->Menu_File(), this);

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

