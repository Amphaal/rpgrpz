#pragma once

#include <QDebug>

#include "src/helpers/_appContext.h"
#include "src/network/youtube/YoutubeHelper.h"
#include "src/network/youtube/YoutubeVideoMetadata.h"
#include "TestMainWindow.h"
#include <QApplication>

int testApp(int argc, char** argv){    
    
    QApplication app(argc, argv);
    AppContext::configureApp(app);

    TestMainWindow test;
    return app.exec();
}

