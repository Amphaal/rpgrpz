#define _HAS_STD_BYTE 0

#include <QtCore/QString>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QSplashScreen>
#include <QImageReader>

#include "src/helpers/_const.hpp"
#include "src/helpers/_messageHandler.hpp"

#include "ui/MainWindow.h"
#include "ui/AppLoader.h"

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

    //configure QThreads to acknowledge specific types for data exchanges
    qRegisterMetaType<std::string>("std::string");

    //message handler
    qInstallMessageHandler(msgHandler::customMO);

    //preload log
    qDebug() << "handled image formats " << QImageReader::supportedImageFormats();

    //fetch main window
    AppLoader loader;
    MainWindow mw;
    loader.finish(&mw);

    //wait for the app to close
    return app.exec();
}