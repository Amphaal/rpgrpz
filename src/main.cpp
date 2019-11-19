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

    QObject::connect(
        &app, &QObject::destroyed,
        [=]() {
            delete AppContext::settings();
        }
    );

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

        //native
        qRegisterMetaType<QPainterPath>("QPainterPath");

        //gst
        qRegisterMetaType<GstMessageType>("GstMessageType");
        qRegisterMetaType<gint64>("gint64");

        //enum
        qRegisterMetaType<RPZJSON::Method>("RPZJSON::Method");
        qRegisterMetaType<Payload::Alteration>("Payload::Alteration");
        qRegisterMetaType<RPZAtom::Parameter>("RPZAtom::Parameter");
        qRegisterMetaType<ProgressTracker::Kind>("ProgressTracker::Kind");
        qRegisterMetaType<RPZStatusLabel::State>("RPZStatusLabel::State");

        //typedef
        qRegisterMetaType<SnowFlake::Id>("SnowFlake::Id");
        qRegisterMetaType<RPZAsset::Hash>("RPZAsset::Hash");
        qRegisterMetaType<RPZAtom::Updates>("RPZAtom::Updates");
        qRegisterMetaType<RPZAtom::ManyUpdates>("RPZAtom::ManyUpdates");

        //QVariantHash derivates
        qRegisterMetaType<AlterationPayload>("AlterationPayload");
        qRegisterMetaType<RPZResponse>("RPZResponse");
        qRegisterMetaType<RPZUser>("RPZUser");
        qRegisterMetaType<RPZUser>("RPZAtom");
        qRegisterMetaType<RPZQuickDraw>("RPZQuickDraw");
        qRegisterMetaType<RPZMessage>("RPZMessage");
        qRegisterMetaType<ResetPayload>("ResetPayload");
        qRegisterMetaType<RPZAssetImportPackage>("RPZAssetImportPackage");
        qRegisterMetaType<RPZCharacter>("RPZCharacter");
        qRegisterMetaType<StreamPlayStateTracker>("StreamPlayStateTracker");
    
        //struct
        qRegisterMetaType<AtomsSelectionDescriptor>("AtomsSelectionDescriptor");

        //containers
        qRegisterMetaType<QVector<RPZUser>>("QVector<RPZUser>");
        qRegisterMetaType<RPZMap<RPZUser>>("RPZMap<RPZUser>");
        qRegisterMetaType<QList<RPZAtom::Id>>("QList<RPZAtom::Id>");
        qRegisterMetaType<QList<QGraphicsItem*>>("QList<QGraphicsItem*>");
        qRegisterMetaType<QList<RPZAsset::Hash>>("QList<RPZAsset::Hash>");
        qRegisterMetaType<QVector<RPZAsset::Hash>>("QVector<RPZAsset::Hash>");
        qRegisterMetaType<QVector<RPZMessage>>("QVector<RPZMessage>");
        qRegisterMetaType<QHash<QGraphicsItem*,RPZAtom::Updates>>("QHash<QGraphicsItem*,RPZAtom::Updates>");
    
    
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
