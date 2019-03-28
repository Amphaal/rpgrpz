#include "src/localization/i18n.cpp"
#include "libs/qtautoupdater/autoupdatercore/updater.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>

#include <QLabel>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSettings>
#include <QIntValidator>

#include "src/network/ConnectivityHelper.h"
#include "src/network/chat/ChatServer.h"
#include "src/network/chat/ChatClient.cpp"

#include "src/ui/components/ChatWidget.cpp"


class MainWindow : public QMainWindow { 
    public:
        MainWindow() {
            
            //preinit...
            this->_initConnectivity();

            //init...
            this->_initUI();
            this->_setupAutoUpdate();

            //initial show
            this->trueShow();
        }

        void trueShow() {
            this->showNormal();
            this->activateWindow();
            this->raise();
        }

        void _initConnectivity() {
            this->_ipHelper = new ConnectivityHelper(this);
            auto cs = new ChatServer();
            cs->start();
        }

    private:
        
        ConnectivityHelper* _ipHelper;
        
        //////////////
        /// UI init //
        //////////////

        void _initUI() {
            
            //values specific to this
            std::string stdTitle = IS_DEBUG_APP ? (std::string)"DEBUG - " + APP_NAME : APP_NAME;
            this->setWindowTitle(QString(stdTitle.c_str()));
            this->setMinimumSize(QSize(480, 400));
            this->setWindowIcon(QIcon(LOCAL_ICON_PNG_PATH.c_str()));

            //central widget
            auto centralW = new QWidget(this);
            centralW->setLayout(new QVBoxLayout);
            centralW->layout()->setAlignment(Qt::AlignTop);
            centralW->layout()->setMargin(0);
            this->setCentralWidget(centralW);

            //specific componements
            this->_initUIMenu();
            this->_initUIConnectionPanel();
            this->_initUIChat();
            this->_initUIStatusBar();
        }   

        QLineEdit* _portTarget = 0;
        QLineEdit* _domainTarget = 0;
        void _initUIConnectionPanel() {
            
            QSettings settings;
            settings.beginGroup("MainWindow");

            auto connectionWdget = new QWidget(this);
            connectionWdget->setLayout(new QHBoxLayout);
            connectionWdget->layout()->setContentsMargins(10, 10, 10, 5);

            this->_domainTarget = new QLineEdit(this);
            this->_domainTarget->setPlaceholderText("IP ou domaine du serveur");
            this->_domainTarget->setText(settings.value("domain", "").toString());

            auto sep = new QLabel(this);
            sep->setText(":");
            sep->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            this->_portTarget = new QLineEdit(this);
            this->_portTarget->setValidator(new QIntValidator(0, 65535));
            this->_portTarget->setPlaceholderText("Port");
            this->_portTarget->setText(
                settings.value(
                    "port", 
                    QString::fromStdString(UPNP_DEFAULT_TARGET_PORT)
                ).toString()
            );
            this->_portTarget->setFixedSize(40, 22);
            this->_portTarget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            auto controlBtn = new QPushButton(this);
            controlBtn->setText("Se connecter");
            QObject::connect(
                controlBtn, &QPushButton::clicked,
                this, &MainWindow::tryConnectToServer
            );

            connectionWdget->layout()->addWidget(this->_domainTarget);
            connectionWdget->layout()->addWidget(sep);
            connectionWdget->layout()->addWidget(this->_portTarget);
            connectionWdget->layout()->addWidget(controlBtn);

            settings.endGroup();
            this->centralWidget()->layout()->addWidget(connectionWdget);
        }

        void _initUIChat() {
            auto chatWdget = new ChatWidget(this);
            this->centralWidget()->layout()->addWidget(chatWdget);
        }

        void _initUIMenu() {
            auto menuBar = new QMenuBar;
            menuBar->addMenu(this->_getFileMenu());
            menuBar->addMenu(this->_getAboutMenu());
            this->setMenuWidget(menuBar);
        }

        void _initUIStatusBar() {
            
            qDebug() << "UI : StatusBar instantiation";

            auto statusBar = new QStatusBar(this);

            auto sb_widget = new QWidget;
            auto extIpDescrLabel = new QLabel("IP externe:");
            auto sep1 = new QLabel(" | ");
            auto localIpDescrLabel = new QLabel("IP locale:");
            auto sep2 = new QLabel(" | ");
            auto upnpDescrLabel = new QLabel("uPnP:");

            //define statusbar content
            sb_widget->setLayout(new QHBoxLayout);
            sb_widget->layout()->addWidget(localIpDescrLabel);
            sb_widget->layout()->addWidget(this->_ipHelper->localIpLabel);
            sb_widget->layout()->addWidget(sep1);
            sb_widget->layout()->addWidget(extIpDescrLabel);
            sb_widget->layout()->addWidget(this->_ipHelper->extIpLabel);
            sb_widget->layout()->addWidget(sep2);
            sb_widget->layout()->addWidget(upnpDescrLabel);
            sb_widget->layout()->addWidget(this->_ipHelper->upnpStateLabel);
            
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

        QAction *versionAction;
        QAction *cfugAction;

        QMenu* _getAboutMenu() {

            QMenu *aboutMenuItem = new QMenu(I18n::tr()->Menu_About().c_str());

            //for checking the upgrades available
            this->cfugAction = new QAction(I18n::tr()->Menu_CheckForUpgrades().c_str(), aboutMenuItem);
                QObject::connect(
                this->cfugAction, &QAction::triggered,
                this, &MainWindow::requireUpdateCheckFromUser
            );
                    
            this->versionAction = new QAction(APP_FULL_DENOM, aboutMenuItem);
            this->versionAction->setEnabled(false);

            aboutMenuItem->addAction(this->cfugAction);
            aboutMenuItem->addSeparator();
            aboutMenuItem->addAction(this->versionAction);

            return aboutMenuItem;
        }

        QMenu* _getFileMenu() {

            auto fileMenuItem = new QMenu(I18n::tr()->Menu_File().c_str());

            //quit
            auto quitAction = new QAction(I18n::tr()->Menu_Quit().c_str(), fileMenuItem);
            QObject::connect(
                quitAction, &QAction::triggered,
                this, &MainWindow::close
            );

            auto openLogAction = new QAction(I18n::tr()->Menu_OpenLog().c_str(), fileMenuItem);
            QObject::connect(
                openLogAction, &QAction::triggered,
                [&]() {
                    openFileInOS(getLogFileLocation());
                }
            );

            auto openLatestLogAction = new QAction(I18n::tr()->Menu_OpenLatestLog().c_str(), fileMenuItem);
            QObject::connect(
                openLatestLogAction, &QAction::triggered,
                [&]() {
                    openFileInOS(getLatestLogFileLocation());
                }
            );

            auto df = getAppDataLocation();
            auto openDataFolderAction = new QAction(I18n::tr()->Menu_OpenDataFolder(df).c_str(), fileMenuItem);
            QObject::connect(
                openDataFolderAction, &QAction::triggered,
                [&, df]() {
                    openFolderInOS(df);
                }
            );


            fileMenuItem->addAction(openLogAction);
            fileMenuItem->addAction(openLatestLogAction);
            fileMenuItem->addSeparator();
            fileMenuItem->addAction(openDataFolderAction);
            fileMenuItem->addSeparator();
            fileMenuItem->addAction(quitAction);

            return fileMenuItem;
        };

        //////////////////////////
        /// END Menu components //
        //////////////////////////
        
        ////////////////////////
        /// Server Connection //
        ////////////////////////

        ChatClient* _cc = 0;
        void tryConnectToServer() {

            //register default values
            QSettings settings;
            settings.beginGroup("MainWindow");

            auto dt_text = this->_domainTarget->text();
            if(!dt_text.isEmpty()) settings.setValue("domain", dt_text);

            auto pt_text = this->_portTarget->text();
            if(!pt_text.isEmpty()) settings.setValue("port", pt_text);          

            settings.endGroup();

            //connect..
            if(this->_cc) delete _cc;
            this->_cc = new ChatClient(dt_text, pt_text, this);

        }

        ////////////////////////////
        /// END Server Connection //
        ////////////////////////////

        ////////////////////
        /// check updates //
        ////////////////////

        QtAutoUpdater::Updater *updater;
        bool userNotificationOnUpdateCheck = false;

        void UpdateSearch_switchUI(bool isSearching) {
            this->cfugAction->setEnabled(!isSearching);
            std::string descr = APP_FULL_DENOM;
            if(isSearching) descr += " - " + I18n::tr()->SearchingForUpdates();
            this->versionAction->setText(descr.c_str());
        }
    
        void _setupAutoUpdate() {
            
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

        
        void onUpdateChecked(bool hasUpdate, bool hasError) {

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

        void requireUpdateCheckFromUser() {

            this->userNotificationOnUpdateCheck = true;

            if (!this->updater->isRunning()) {
                this->checkForAppUpdates();
            }
        };

        void checkForAppUpdates() {
            this->UpdateSearch_switchUI(true);
            this->updater->checkForUpdates();
        }

        ////////////////////////
        /// END check updates //
        ////////////////////////
};