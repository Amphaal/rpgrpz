#include "MainWindow.h"


MainWindow::MainWindow() {
    
    //init...
    this->_initUI();
    this->_setupAutoUpdate();
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
    
    if (this->_chatServer && this->_chatServer->isRunning()) {
        this->_chatServer->exit();
        this->_chatServer->wait();
    }

    event->accept();
}


void MainWindow::_initConnectivity() {
    
    ////////////////////////////
    /// Connectivity helper ! //
    ////////////////////////////
    
    this->_ipHelper = new ConnectivityHelper(this);

    QObject::connect(this->_ipHelper, &ConnectivityHelper::localAddressStateChanged, this, &MainWindow::updateIntIPLabel);
    QObject::connect(this->_ipHelper, &ConnectivityHelper::remoteAddressStateChanged, this, &MainWindow::updateExtIPLabel);
    QObject::connect(this->_ipHelper, &ConnectivityHelper::uPnPStateChanged, this, &MainWindow::updateUPnPLabel);

    this->_ipHelper->init();

    ////////////////////
    /// Chat Server ! //
    ////////////////////

    this->_chatServer = new RPZServer;
    this->_chatServer->start();
}

void MainWindow::updateUPnPLabel(std::string state) {
    this->_upnpStateLabel->setText(QString::fromStdString(state));
}

void MainWindow::updateExtIPLabel(std::string state, bool isOn) {
    
    this->_extIpLabel->setText(QString::fromStdString(state));

    if(isOn) {
        this->_extIpLabel->setText("<a href='" + this->_extIpLabel->text() + "'>" + this->_extIpLabel->text() + "</a>");
    }
}

void MainWindow::updateIntIPLabel(std::string state) {
    this->_localIpLabel->setText(QString::fromStdString(state));
}

//////////////
/// UI init //
//////////////

void MainWindow::_initUI() {
    
    //values specific to this
    std::string stdTitle = IS_DEBUG_APP ? (std::string)"DEBUG - " + APP_FULL_DENOM : APP_FULL_DENOM;
    this->setWindowTitle(QString(stdTitle.c_str()));

    this->setWindowIcon(QIcon(":/icons/app/rpgrpz.png"));

    //central widget
    auto centralW = new RestoringSplitter("MainWindowSplitter");
    centralW->setContentsMargins(10, 5, 10, 10);
    this->setCentralWidget(centralW);

    //specific componements
    this->_initUIMenu();
    this->_initUIStatusBar();
    this->_initUIApp();
}   

void MainWindow::_initUIApp() {
    
    //init components
    this->_connectWidget = new ConnectWidget(this);
    this->_cw = new ChatWidget(this);
    this->_mapView = new MapView(this);
    this->_streamNotifier = new AudioStreamNotifier(this);
    this->_assetsManager = new AssetsManager(this);
    this->_mapTools = new MapTools(this);

    //place them...
    
    //assets
    auto tabs = new QTabWidget(this);
    tabs->addTab(this->_assetsManager, "Boite à jouets");
    tabs->addTab(new QWidget, "Carte");

    //designer
    auto designer = new QWidget();
    designer->setLayout(new QVBoxLayout);
    designer->layout()->setMargin(0);
    designer->layout()->setSpacing(2);
    designer->layout()->addWidget(this->_mapTools);
    designer->layout()->addWidget(this->_mapView);

    //Chat...
    auto right = new QWidget;
    right->setLayout(new QVBoxLayout);
    right->layout()->setMargin(0);
    right->layout()->addWidget(this->_connectWidget);
    right->layout()->addWidget(this->_cw);
    right->layout()->addWidget(this->_streamNotifier);


    //final
    auto centralWidget = (RestoringSplitter*)this->centralWidget();
    centralWidget->addWidget(tabs);
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
        [&](RPZClient * cc) {
            this->_cw->bindToRPZClient(cc);
        }
    );

    //move to map content tab when selection changed inside the map
    QObject::connect(
        this->_mapView->scene(), &QGraphicsScene::selectionChanged,
        [tabs]() {
            tabs->setCurrentIndex(1);
        }
    );

    //unselect tools
    QObject::connect(
        this->_mapView, &MapView::unselectCurrentToolAsked,
        this->_mapTools, &MapTools::unselectAllTools
    );

    //bind toolbar to mapview
    QObject::connect(
        this->_mapTools, &QToolBar::actionTriggered,
        this->_mapView, &MapView::toolSelectionChanged
    );

}

void MainWindow::_initUIMenu() {
    auto menuBar = new QMenuBar;
    menuBar->addMenu(this->_getFileMenu());
    menuBar->addMenu(this->_getHelpMenu());
    this->setMenuWidget(menuBar);
}

void MainWindow::_initUIStatusBar() {
    
    qDebug() << "UI : StatusBar instantiation";

    auto statusBar = new QStatusBar(this);

    auto sb_widget = new QWidget;
    auto extIpDescrLabel = new QLabel("IP externe:");
    auto sep1 = new QLabel(" | ");
    auto localIpDescrLabel = new QLabel("IP locale:");
    auto sep2 = new QLabel(" | ");
    auto upnpDescrLabel = new QLabel("uPnP:");

    auto syncMsg = "<Synchronisation...>";
    this->_localIpLabel = new QLabel(syncMsg);
    this->_extIpLabel = new QLabel(syncMsg);
    this->_upnpStateLabel = new QLabel(syncMsg);

    QObject::connect(this->_extIpLabel, &QLabel::linkActivated, [&]() {
        
        //remove html tags
        auto s = this->_extIpLabel->text();
        s.remove(QRegExp("<[^>]*>"));
        QApplication::clipboard()->setText(s);

        //show tooltip
        QToolTip::showText(QCursor::pos(), "IP copiée !");

    });

    //define statusbar content
    auto colors = statusBar->palette();
    statusBar->setAutoFillBackground(true);
    colors.setColor(QPalette::Background, "#DDD");
    statusBar->setPalette(colors);

    sb_widget->setLayout(new QHBoxLayout);
    sb_widget->layout()->addWidget(localIpDescrLabel);
    sb_widget->layout()->addWidget(this->_localIpLabel);
    sb_widget->layout()->addWidget(sep1);
    sb_widget->layout()->addWidget(extIpDescrLabel);
    sb_widget->layout()->addWidget(this->_extIpLabel);
    sb_widget->layout()->addWidget(sep2);
    sb_widget->layout()->addWidget(upnpDescrLabel);
    sb_widget->layout()->addWidget(this->_upnpStateLabel);
    
    sb_widget->layout()->setContentsMargins(10, 5, 5, 5);
    
    //define statusbar
    statusBar->addWidget(sb_widget);
    this->setStatusBar(statusBar);
}

//////////////////
/// END UI init //
//////////////////

//////////////////////
/// Menu components //
//////////////////////

QMenu* MainWindow::_getHelpMenu() {

    auto helpMenuItem = new QMenu(I18n::tr()->Menu_Help().c_str());

    //full log
    auto openLogAction = new QAction(I18n::tr()->Menu_OpenLog().c_str(), helpMenuItem);
    QObject::connect(
        openLogAction, &QAction::triggered,
        [&]() {
            openFileInOS(getLogFileLocation());
        }
    );

    //latest log
    auto openLatestLogAction = new QAction(I18n::tr()->Menu_OpenLatestLog().c_str(), helpMenuItem);
    QObject::connect(
        openLatestLogAction, &QAction::triggered,
        [&]() {
            openFileInOS(getLatestLogFileLocation());
        }
    );

    //data folder
    auto df = getAppDataLocation();
    auto openDataFolderAction = new QAction(I18n::tr()->Menu_OpenDataFolder(df).c_str(), helpMenuItem);
    QObject::connect(
        openDataFolderAction, &QAction::triggered,
        [&, df]() {
            openFolderInOS(df);
        }
    );

    //for checking the upgrades available
    this->cfugAction = new QAction(I18n::tr()->Menu_CheckForUpgrades().c_str(), helpMenuItem);
    QObject::connect(
        this->cfugAction, &QAction::triggered,
        this, &MainWindow::requireUpdateCheckFromUser
    );

    //patchnote
    auto patchnoteAction = new QAction(I18n::tr()->Menu_Patchnotes(APP_FULL_DENOM).c_str(), helpMenuItem);
    QObject::connect(
        patchnoteAction, &QAction::triggered,
        [&]() {
            QDesktopServices::openUrl(QUrl(APP_PATCHNOTE_URL));
        }
    );

    helpMenuItem->addAction(openDataFolderAction);
    helpMenuItem->addSeparator();
    helpMenuItem->addAction(openLogAction);
    helpMenuItem->addAction(openLatestLogAction);
    helpMenuItem->addSeparator();
    helpMenuItem->addAction(patchnoteAction);
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


////////////////////
/// check updates //
////////////////////

void MainWindow::UpdateSearch_switchUI(bool isSearching) {
    this->cfugAction->setEnabled(!isSearching);
    std::string descr = isSearching ? I18n::tr()->SearchingForUpdates() : I18n::tr()->Menu_CheckForUpgrades();
    this->cfugAction->setText(descr.c_str());
}

void MainWindow::_setupAutoUpdate() {
    
    qDebug() << "UI : AutoUpdate instantiation";

    if(MAINTENANCE_TOOL_LOCATION == "") {
        this->updater = new QtAutoUpdater::Updater(this);
    }
    else {
        this->updater = new QtAutoUpdater::Updater(MAINTENANCE_TOOL_LOCATION, this);
    }

    QObject::connect(this->updater, &QtAutoUpdater::Updater::checkUpdatesDone, 
                    this, &MainWindow::onUpdateChecked);

    //start the update check
    this->checkForAppUpdates();
}


void MainWindow::onUpdateChecked(bool hasUpdate, bool hasError) {

    //if the user asks directly to check updates
    if(this->userNotificationOnUpdateCheck) {
        this->userNotificationOnUpdateCheck = false;
        
        std::string title = (std::string)APP_NAME + " - " + I18n::tr()->Menu_CheckForUpgrades();
        std::string content = this->updater->errorLog().toStdString();

        if(!hasUpdate && !hasError) {
            QMessageBox::information(this, 
                QString(title.c_str()), 
                QString(content.c_str()), 
                QMessageBox::Ok, QMessageBox::Ok);
        } else if (hasError) {
            QMessageBox::warning(this, 
                QString(title.c_str()), 
                QString(content.c_str()), 
                QMessageBox::Ok, QMessageBox::Ok);
        }
    }

    //no update, no go
    if(!hasUpdate) {
        this->UpdateSearch_switchUI(false);
        return;
    }

    //if has update
    std::string title = (std::string)APP_NAME + " - " + I18n::tr()->Alert_UpdateAvailable_Title();
    std::string content = I18n::tr()->Alert_UpdateAvailable_Text();

    auto msgboxRslt = QMessageBox::information(this, 
                QString(title.c_str()), 
                QString(content.c_str()), 
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
    );
    
    if(msgboxRslt == QMessageBox::Yes) {
        this->updater->runUpdaterOnExit();
        this->close();
    }

    this->UpdateSearch_switchUI(false);
};

void MainWindow::requireUpdateCheckFromUser() {

    this->userNotificationOnUpdateCheck = true;

    if (!this->updater->isRunning()) {
        this->checkForAppUpdates();
    }
};

void MainWindow::checkForAppUpdates() {
    this->UpdateSearch_switchUI(true);
    this->updater->checkForUpdates();
}

////////////////////////
/// END check updates //
////////////////////////