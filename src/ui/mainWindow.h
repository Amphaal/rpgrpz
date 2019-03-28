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
#include "src/network/chat/server/ChatServer.h"
#include "src/network/chat/client/ChatClient.cpp"

#include "src/ui/components/ChatWidget.cpp"


class MainWindow : public QMainWindow { 
    public:
        MainWindow();

    private:
        void trueShow();

        void updateUPnPLabel(std::string state);
        void updateExtIPLabel(std::string state);
        void updateIntIPLabel(std::string state);

        ConnectivityHelper* _ipHelper;
        void _initUI();
        void _initConnectivity();

        QLineEdit* _portTarget = 0;
        QLineEdit* _domainTarget = 0;
        void _initUIConnectionPanel();
        
        ChatWidget* _cw = 0;
        void _initUIChat();
        void onNewConnectionFromServer(std::string clientIp);

        void _initUIMenu();
        
        QLabel* _localIpLabel;
        QLabel* _extIpLabel;
        QLabel* _upnpStateLabel;
        void _initUIStatusBar();

        QAction *versionAction;
        QAction *cfugAction;

        QMenu* _getAboutMenu();
        QMenu* _getFileMenu();

        ChatClient* _cc = 0;
        void tryConnectToServer();

        QtAutoUpdater::Updater *updater;
        bool userNotificationOnUpdateCheck = false;
        void UpdateSearch_switchUI(bool isSearching);
        void _setupAutoUpdate();
        void onUpdateChecked(bool hasUpdate, bool hasError);
        void requireUpdateCheckFromUser();;
        void checkForAppUpdates();
};