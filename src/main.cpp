#define _HAS_STD_BYTE 0

#include <QString>
#include <QApplication>
#include <QStyleFactory>
#include <QSplashScreen>
#include <QImageReader>

#include "src/helpers/_const.hpp"
#include "src/helpers/_logWriter.hpp"

#include "src/network/rpz/server/RPZServer.h"

#include "ui/MainWindow.h"
#include "ui/AppLoader.hpp"

#include <QDir>
#include <QLockFile>

#include "tuto.hpp"

////////////
// SERVER //
////////////

int serverConsole(int argc, char** argv) {
    
    QCoreApplication server(argc, argv);
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
    app.setApplicationName(QString(APP_NAME));
    app.setOrganizationName(QString(APP_PUBLISHER));
    app.setStyle(QStyleFactory::create("Fusion")); 
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

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

    //configure QThreads to acknowledge specific types for data exchanges
    qRegisterMetaType<std::string>("std::string");

    //message handler
    qInstallMessageHandler(LogWriter::customMO);

    ////////////
    // LAUNCH //
    ////////////

    return tuto(argc, argv);

    if(QString(argv[1]) == "serverOnly") {
        
        //as server console
        return serverConsole(argc, argv);

    } else {

        //as client app
        return clientApp(argc, argv);

    }

}
