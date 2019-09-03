#include <QDebug>
#include <QString>
#include <QApplication>
#include <QStyleFactory>
#include <QSplashScreen>
#include <QImageReader>
#include <QLockFile>
#include <QPixmapCache>
#include <QSslSocket>

#include "src/helpers/_appContext.h"
#include "src/helpers/_logWriter.h"

#include "src/network/rpz/server/RPZServer.h"

#include "ui/mainWindow.h"
#include "ui/AppLoader.h"

#include "_tests/tests.hpp"

#include <QDir>
#include <QtCore/QLockFile>

#include "_libs/snowflake/snowflake.h"

#include "version.h"

////////////
// SERVER //
////////////

int serverConsole(int argc, char** argv) {
    
    QCoreApplication server(argc, argv);
    AppContext::configureApp(server);

    RPZServer app;
    app.run();

    return server.exec();
}

////////////////
// END SERVER //
////////////////

////////////
// CLIENT //
////////////

int clientApp(int argc, char** argv) {

    if(!IS_DEBUG_APP) {
        // prevent multiples instances
        QString tmpDir = QDir::tempPath();
        QLockFile lockFile(tmpDir + "/rpgrpz.lock");
        if(!lockFile.tryLock(100)){
            return 1;
        }
    }
    
    //setup app
    QApplication app(argc, argv);
    AppContext::configureApp(app);

    app.setApplicationDisplayName(APP_NAME);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setStyle(QStyleFactory::create("Fusion")); 

    //set cache limit to max
    QPixmapCache::setCacheLimit(INT_MAX);

    //fetch main window
    AppLoader loader;
    MainWindow mw;
    loader.finish(&mw);

    //wait for the app to close
    return app.exec();
}

////////////////
// END CLIENT //
////////////////

int main(int argc, char** argv) {

    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::sslLibraryVersionString();

    //message handler
    qInstallMessageHandler(LogWriter::customMO);

    ////////////
    // LAUNCH //
    ////////////

    //default
    auto args = AppContext::getOptionArgs(argc, argv);

    //conditionnal
    if(args.contains("test")) {
        
        //test app
        return testApp(argc, argv);

    } 
    
    else if (args.contains("serverOnly")) {
        
        //as server console
        return serverConsole(argc, argv);

    }

    //as client app
    return clientApp(argc, argv);

}
