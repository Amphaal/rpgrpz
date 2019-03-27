#define _HAS_STD_BYTE 0

#include <QtCore/QString>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

#include "src/helpers/_const.cpp"
#include "src/helpers/_messageHandler.cpp"

#include "ui/mainWindow.cpp"

#include <QDir>
#include <QLockFile>

int main(int argc, char** argv){
    
    //prevent multiples instances
    QString tmpDir = QDir::tempPath();
    QLockFile lockFile(tmpDir + "/rpgrpz.lock");
    if(!lockFile.tryLock(100)){
        return 1;
    }


    //setup app
    QApplication app(argc, argv);
    app.setApplicationName(QString(APP_NAME));
    app.setOrganizationName(QString(APP_PUBLISHER));
    app.setStyle(QStyleFactory::create("Fusion")); 
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    //message handler
    qInstallMessageHandler(msgHandler::customMO);

    //fetch main window
    MainWindow mw;

    //wait for the app to close
    return app.exec();
}