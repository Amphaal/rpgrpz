#include <QLockFile>
#include <QStyleFactory>

#include "src/ui/mainWindow.h"
#include "src/ui/_others/AppLoader.hpp"
#include "src/helpers/_logWriter.h"

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
    AppContext::installTranslations(app);
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

    //log SLL lib loading
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::sslLibraryVersionString();   

    //registering metatypes
    qRegisterMetaType<JSONMethod>("JSONMethod");
    qRegisterMetaType<AlterationPayload>("AlterationPayload");
    qRegisterMetaType<PayloadAlteration>("PayloadAlteration");
    qRegisterMetaType<AtomUpdates>("AtomUpdates");
    qRegisterMetaType<AtomsUpdates>("AtomsUpdates");
    qRegisterMetaType<RPZUser>("RPZUser");
    qRegisterMetaType<AtomParameter>("AtomParameter");
    qRegisterMetaType<RPZMessage>("RPZMessage");
    qRegisterMetaType<RPZResponse>("RPZResponse");
    qRegisterMetaType<RPZAssetImportPackage>("RPZAssetImportPackage");
    qRegisterMetaType<QList<QTreeWidgetItem*>>("QList<QTreeWidgetItem*>");
    qRegisterMetaType<QList<QGraphicsItem*>>("QList<QGraphicsItem*>");
    qRegisterMetaType<ResetPayload>("ResetPayload");
    qRegisterMetaType<QVector<RPZAtomId>>("QVector<RPZAtomId>");
    qRegisterMetaType<QVector<RPZUser>>("QVector<RPZUser>");
    qRegisterMetaType<RPZMap<RPZUser>>("RPZMap<RPZUser>");
    qRegisterMetaType<QVector<RPZMessage>>("QVector<RPZMessage>");
    qRegisterMetaType<QHash<QTreeWidgetItem*,AtomUpdates>>("QHash<QTreeWidgetItem*,AtomUpdates>");
    qRegisterMetaType<QHash<QGraphicsItem*,AtomUpdates>>("QHash<QGraphicsItem*,AtomUpdates>");
    qRegisterMetaType<AtomsSelectionDescriptor>("AtomsSelectionDescriptor");
    qRegisterMetaType<QHash<int,QList<QTreeWidgetItem*>>>("QHash<int,QList<QTreeWidgetItem*>>");
    qRegisterMetaType<RPZStatusLabel::State>("RPZStatusLabel::State");
    qRegisterMetaType<QList<RPZAssetHash>>("QList<RPZAssetHash>");
    qRegisterMetaType<ProgressTracker::Kind>("ProgressTracker::Kind");
    qRegisterMetaType<QVector<RPZAssetHash>>("QVector<RPZAssetHash>");
    qRegisterMetaType<GstMessageType>("GstMessageType");
    qRegisterMetaType<StreamPlayStateTracker>("StreamPlayStateTracker");
    qRegisterMetaType<gint64>("gint64");
    qRegisterMetaType<snowflake_uid>("snowflake_uid");
    qRegisterMetaType<RPZCharacter>("RPZCharacter");

    //message handler
    qInstallMessageHandler(LogWriter::customMO);

    ////////////
    // LAUNCH //
    ////////////

    //default
    auto args = AppContext::getOptionArgs(argc, argv);

    if (args.contains(QStringLiteral(u"serverOnly"))) {
        
        //as server console
        return serverConsole(argc, argv);

    }

    //as client app
    return clientApp(argc, argv);

}
