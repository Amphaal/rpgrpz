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

    // auto video = YoutubeVideoMetadata::fromUrl("https://www.youtube.com/watch?time_continue=211&v=6p03KD5IBrI");
    // YoutubeHelper::refreshMetadata(video).then([=]() {
        
    //     auto test1 = QString("https://zonme.to2x.ovh/test.mp3");
    //     auto test2 = QString("https://zonme.to2x.ovh/test2.mp4");
    //     auto url = video->audioStreams()->getPreferedMineSourcePair().second;
    //     auto i = url.toStdString();
    //     auto qq = true;
    // });

    TestMainWindow test;
    return app.exec();
}

