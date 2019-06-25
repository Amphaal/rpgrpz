#pragma once
#include <QDebug>

#include <QString>
#include <QApplication>
#include <QStyleFactory>
#include <QSplashScreen>
#include <QImageReader>

#include "src/helpers/_appContext.h"
#include "src/helpers/_logWriter.h"

#include "src/network/rpz/server/RPZServer.h"

#include "ui/MainWindow.h"
#include "ui/AppLoader.h"

#include "_tests/tests.hpp"

#include <QDir>
#include <QLockFile>

#include "_libs/snowflake.h"


////////////
// SERVER //
////////////

int serverConsole(int argc, char** argv) {
    
    QCoreApplication server(argc, argv);
    AppContext::configureApp(server);

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
    AppContext::configureApp(app);

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
            return testApp(argc, argv);


        } else if (typeLaunch == "serverOnly") {
            
            //as server console
            return serverConsole(argc, argv);

        }

    } 

    //as client app
    return clientApp(argc, argv);

}
