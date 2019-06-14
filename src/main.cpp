#define _HAS_STD_BYTE 0

#include <QDebug>

#include <QString>
#include <QApplication>
#include <QStyleFactory>
#include <QSplashScreen>
#include <QImageReader>

#include "src/helpers/_appContext.h"
#include "src/helpers/_logWriter.hpp"

#include "src/network/rpz/server/RPZServer.h"

#include "ui/MainWindow.h"
#include "ui/AppLoader.hpp"

#include "_tests/TestMainWindow.h"

#include <QDir>
#include <QLockFile>

#include "_libs/snowflake.hpp"

void configureApp(QCoreApplication &app) {
    
    //context preparation
    app.setApplicationName(QString(APP_NAME));
    app.setOrganizationName(QString(APP_PUBLISHER));
    
    //define context
    auto args = AppContext::getOptionArgs(app);
    QString customContext = NULL;
    
    //if custom context is set
    if(args.count() > 1) {

        customContext = args[1];
        if(customContext == "random") {
            return AppContext::initRandomContext();
        } else {
            return AppContext::initCustomContext(customContext);
        }
    }

    AppContext::init();
}

////////////
// SERVER //
////////////

int serverConsole(int argc, char** argv) {
    
    QCoreApplication server(argc, argv);
    configureApp(server);

    auto rpz = new RPZServer;
    rpz->run();

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
        //prevent multiples instances
        QString tmpDir = QDir::tempPath();
        QLockFile lockFile(tmpDir + "/rpgrpz.lock");
        if(!lockFile.tryLock(100)){
            return 1;
        }
    }

    //setup app
    QApplication app(argc, argv);
    configureApp(app);

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setStyle(QStyleFactory::create("Fusion")); 

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

//////////
// TEST //
//////////

int test(int argc, char** argv){    
    QApplication app(argc, argv);
    configureApp(app);
    TestMainWindow test;
    return app.exec();
}

//////////////
// END TEST //
//////////////

int main(int argc, char** argv){

    //message handler
    qInstallMessageHandler(LogWriter::customMO);

    ////////////
    // LAUNCH //
    ////////////

    //default
    auto args = AppContext::getOptionArgs(argc, argv);
    if(args.count()) {

        //conditionnal
        auto typeLaunch = args[0];
        if(typeLaunch == "test") {
            
            //test app
            return test(argc, argv);


        } else if (typeLaunch == "serverOnly") {
            
            //as server console
            return serverConsole(argc, argv);

        }

    } 

    //as client app
    return clientApp(argc, argv);

}
