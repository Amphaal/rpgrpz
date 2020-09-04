// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include <QLockFile>
#include <QStyleFactory>

#include <exception>
#include <iostream>

#include "src/ui/mainWindow.h"
#include "src/ui/_others/AppLoader.hpp"

////////////
// SERVER //
////////////

int serverConsole(int argc, char** argv) {
    QCoreApplication server(argc, argv);
    AppContext::configureApp(&server);
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
    #ifndef _DEBUG

        auto lockFn = QString("%1/%2.lock")
                            .arg(QDir::tempPath())
                            .arg(APP_NAME);
        QLockFile lockFile(lockFn);

        if (!lockFile.tryLock(100)) {
            return 1;
        }

    #endif

    // setup app
    QApplication app(argc, argv);
    AppContext::configureApp(&app);
    AppContext::installTranslations(&app);

    QObject::connect(
        &app, &QObject::destroyed,
        [=]() {
            delete AppContext::settings();
    });

    app.setApplicationDisplayName(AppContext::getWindowTitle());
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setStyle(QStyleFactory::create("Fusion"));

    // set cache limit to max
    QPixmapCache::setCacheLimit(INT_MAX);

    // fetch main window
    AppLoader loader;
    MainWindow mw;
    loader.finish(&mw);

    // wait for the app to close
    return app.exec();
}

////////////////
// END CLIENT //
////////////////

void _registerMetaTypes() {
    // native
    qRegisterMetaType<QPainterPath>("QPainterPath");
    // gst
    qRegisterMetaType<GstMessageType>("GstMessageType");
    qRegisterMetaType<gint64>("gint64");
    // enum
    qRegisterMetaType<RPZJSON::Method>("RPZJSON::Method");
    qRegisterMetaType<Payload::Alteration>("Payload::Alteration");
    qRegisterMetaType<RPZAtom::Parameter>("RPZAtom::Parameter");
    qRegisterMetaType<RPZStatusLabel::State>("RPZStatusLabel::State");
    qRegisterMetaType<RPZFogParams::Mode>("RPZFogParams::Mode");
    // typedef
    qRegisterMetaType<SnowFlake::Id>("SnowFlake::Id");
    qRegisterMetaType<RPZCharacter::Id>("RPZCharacter::Id");
    qRegisterMetaType<RPZAsset::Hash>("RPZAsset::Hash");
    qRegisterMetaType<RPZAtom::Updates>("RPZAtom::Updates");
    qRegisterMetaType<RPZAtom::ManyUpdates>("RPZAtom::ManyUpdates");
    qRegisterMetaType<RPZSharedDocument::FileHash>
        ("RPZSharedDocument::FileHash");
    qRegisterMetaType<RPZSharedDocument::DocumentName>
        ("RPZSharedDocument::DocumentName");
    qRegisterMetaType<RPZSharedDocument::NamesStore>
        ("RPZSharedDocument::NamesStore");
    qRegisterMetaType<CharacterPicker::SelectedCharacter>
        ("CharacterPicker::SelectedCharacter");
    // QVariantHash derivates
    qRegisterMetaType<AlterationPayload>("AlterationPayload");
    qRegisterMetaType<RPZResponse>("RPZResponse");
    qRegisterMetaType<RPZUser>("RPZUser");
    qRegisterMetaType<RPZAtom>("RPZAtom");
    qRegisterMetaType<RPZQuickDrawBits>("RPZQuickDrawBits");
    qRegisterMetaType<RPZMessage>("RPZMessage");
    qRegisterMetaType<ResetPayload>("ResetPayload");
    qRegisterMetaType<RPZGameSession>("RPZGameSession");
    qRegisterMetaType<RPZAssetImportPackage>("RPZAssetImportPackage");
    qRegisterMetaType<RPZCharacter>("RPZCharacter");
    qRegisterMetaType<StreamPlayStateTracker>("StreamPlayStateTracker");
    qRegisterMetaType<RPZFogParams>("RPZFogParams");
    qRegisterMetaType<RPZMapParameters>("RPZMapParameters");
    qRegisterMetaType<RPZSharedDocument>("RPZSharedDocument");
    qRegisterMetaType<RPZPing>("RPZPing");

    // struct
    qRegisterMetaType<AtomsSelectionDescriptor>("AtomsSelectionDescriptor");
    // containers
    qRegisterMetaType<RPZMap<RPZUser>>("RPZMap<RPZUser>");
    qRegisterMetaType<QList<RPZAtom::Id>>("QList<RPZAtom::Id>");
    qRegisterMetaType<QList<QGraphicsItem*>>("QList<QGraphicsItem*>");
    qRegisterMetaType<OrderedGraphicsItems>("OrderedGraphicsItems");
    qRegisterMetaType<QList<RPZAsset::Hash>>("QList<RPZAsset::Hash>");
    qRegisterMetaType<QVector<RPZAsset::Hash>>("QVector<RPZAsset::Hash>");
    qRegisterMetaType<QVector<RPZMessage>>("QVector<RPZMessage>");
    qRegisterMetaType<QHash<QGraphicsItem*, RPZAtom::Updates>>
        ("QHash<QGraphicsItem*,RPZAtom::Updates>");
    qRegisterMetaType<QList<QPolygonF>>("QList<QPolygonF>");
}

int main(int argc, char** argv) {
    // registering metatypes
    _registerMetaTypes();

    ////////////
    // LAUNCH //
    ////////////

    auto args = AppContext::getOptionArgs(argc, argv);

    auto result = args.contains(QStringLiteral(u"serverOnly")) ?
                    serverConsole(argc, argv) :
                    clientApp(argc, argv);

    // make sure to flush sentry for warnings / safe errors
    // sentry_shutdown();

    return result;
}
