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
    qRegisterMetaType<RPZJSON::Method>("RPZJSON::Method");
    qRegisterMetaType<AlterationPayload>("AlterationPayload");
    qRegisterMetaType<Payload::Alteration>("Payload::Alteration");
    qRegisterMetaType<RPZAtom::Updates>("RPZAtom::Updates");
    qRegisterMetaType<RPZAtom::ManyUpdates>("RPZAtom::ManyUpdates");
    qRegisterMetaType<RPZUser>("RPZUser");
    qRegisterMetaType<RPZAtom::Parameter>("RPZAtom::Parameter");
    qRegisterMetaType<RPZMessage>("RPZMessage");
    qRegisterMetaType<RPZResponse>("RPZResponse");
    qRegisterMetaType<RPZAssetImportPackage>("RPZAssetImportPackage");
    qRegisterMetaType<QList<QGraphicsItem*>>("QList<QGraphicsItem*>");
    qRegisterMetaType<ResetPayload>("ResetPayload");
    qRegisterMetaType<QList<RPZAtom::Id>>("QList<RPZAtom::Id>");
    qRegisterMetaType<QVector<RPZUser>>("QVector<RPZUser>");
    qRegisterMetaType<RPZMap<RPZUser>>("RPZMap<RPZUser>");
    qRegisterMetaType<QVector<RPZMessage>>("QVector<RPZMessage>");
    qRegisterMetaType<QHash<QGraphicsItem*,RPZAtom::Updates>>("QHash<QGraphicsItem*,RPZAtom::Updates>");
    qRegisterMetaType<AtomsSelectionDescriptor>("AtomsSelectionDescriptor");
    qRegisterMetaType<RPZStatusLabel::State>("RPZStatusLabel::State");
    qRegisterMetaType<QList<RPZAsset::Hash>>("QList<RPZAsset::Hash>");
    qRegisterMetaType<ProgressTracker::Kind>("ProgressTracker::Kind");
    qRegisterMetaType<QVector<RPZAsset::Hash>>("QVector<RPZAsset::Hash>");
    qRegisterMetaType<GstMessageType>("GstMessageType");
    qRegisterMetaType<StreamPlayStateTracker>("StreamPlayStateTracker");
    qRegisterMetaType<gint64>("gint64");
    qRegisterMetaType<SnowFlake::Id>("SnowFlake::Id");
    qRegisterMetaType<RPZCharacter>("RPZCharacter");
    qRegisterMetaType<QPainterPath>("QPainterPath");

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
