#include <QtWidgets/QMainWindow>

class MainWindow : public QMainWindow { 
    public:
        MainWindow() {

            //values specific to this
            std::string stdTitle = IS_DEBUG_APP ? (std::string)"DEBUG - " + APP_NAME : APP_NAME;
            this->setWindowTitle(QString(stdTitle.c_str()));
            this->setMinimumSize(QSize(480, 400));
            this->setWindowIcon(QIcon(LOCAL_ICON_PNG_PATH.c_str()));

            //initial show
            this->showNormal();
            this->activateWindow();
            this->raise();
        }
}