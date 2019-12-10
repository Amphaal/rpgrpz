#include <QLockFile>
#include <QStyleFactory>

#include "src/ui/mainWindow.h"
#include "src/ui/_others/AppLoader.hpp"
#include "src/helpers/_logWriter.h"

#include <exception>
#include <iostream>

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

    // prevent multiples instances
    #ifndef NDEBUG

        auto lockFn = QString("%1/%2.lock")
                            .arg(QDir::tempPath())
                            .arg(APP_NAME);
        QLockFile lockFile(lockFn);
                            
        if(!lockFile.tryLock(100)){
            return 1;
        }

    #endif
    
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

void _registerMetaTypes() {
   
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
    qRegisterMetaType<SnowFlake::Id>("RPZCharacter::Id");
    qRegisterMetaType<RPZAsset::Hash>("RPZAsset::Hash");
    qRegisterMetaType<RPZAtom::Updates>("RPZAtom::Updates");
    qRegisterMetaType<RPZAtom::ManyUpdates>("RPZAtom::ManyUpdates");
    qRegisterMetaType<CharacterPicker::SelectedCharacter>("CharacterPicker::SelectedCharacter");
    
    //QVariantHash derivates
    qRegisterMetaType<AlterationPayload>("AlterationPayload");
    qRegisterMetaType<RPZResponse>("RPZResponse");
    qRegisterMetaType<RPZUser>("RPZUser");
    qRegisterMetaType<RPZUser>("RPZAtom");
    qRegisterMetaType<RPZQuickDraw>("RPZQuickDraw");
    qRegisterMetaType<RPZMessage>("RPZMessage");
    qRegisterMetaType<ResetPayload>("ResetPayload");
    qRegisterMetaType<RPZGameSession>("RPZGameSession");
    qRegisterMetaType<RPZAssetImportPackage>("RPZAssetImportPackage");
    qRegisterMetaType<RPZCharacter>("RPZCharacter");
    qRegisterMetaType<StreamPlayStateTracker>("StreamPlayStateTracker");
    qRegisterMetaType<RPZFogParams>("RPZFogParams");
    qRegisterMetaType<RPZMapParameters>("RPZMapParameters");
    
    //struct
    qRegisterMetaType<AtomsSelectionDescriptor>("AtomsSelectionDescriptor");

    //containers
    qRegisterMetaType<RPZMap<RPZUser>>("RPZMap<RPZUser>");
    qRegisterMetaType<QList<RPZAtom::Id>>("QList<RPZAtom::Id>");
    qRegisterMetaType<QList<QGraphicsItem*>>("QList<QGraphicsItem*>");
    qRegisterMetaType<OrderedGraphicsItems>("OrderedGraphicsItems");
    qRegisterMetaType<QList<RPZAsset::Hash>>("QList<RPZAsset::Hash>");
    qRegisterMetaType<QVector<RPZAsset::Hash>>("QVector<RPZAsset::Hash>");
    qRegisterMetaType<QVector<RPZMessage>>("QVector<RPZMessage>");
    qRegisterMetaType<QHash<QGraphicsItem*,RPZAtom::Updates>>("QHash<QGraphicsItem*,RPZAtom::Updates>");
    
}

int main(int argc, char** argv) noexcept {

    //log SLL lib loading
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::sslLibraryVersionString();   

    //registering metatypes
    _registerMetaTypes();
     
    //message handler
    qInstallMessageHandler(LogWriter::customMO);

    ////////////
    // LAUNCH //
    ////////////

    try {
        
        auto args = AppContext::getOptionArgs(argc, argv);
        auto result = args.contains(QStringLiteral(u"serverOnly")) ? 
                        serverConsole(argc, argv) : 
                        clientApp(argc, argv);
        
        sentry_shutdown();
        return result;

    } catch(...) {
        sentry_shutdown();
    }

}
