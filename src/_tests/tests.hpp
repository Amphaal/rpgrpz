#pragma once

#include <QDebug>

#include "src/helpers/_appContext.h"
#include "src/network/youtube/YoutubeHelper.h"
#include "src/network/youtube/YoutubeVideoMetadata.h"
#include "TestMainWindow.h"
#include <QApplication>

#include <QMediaPlayer>

int testApp(int argc, char** argv){    
    
    QApplication app(argc, argv);
    AppContext::configureApp(app);

    auto player = new QMediaPlayer(nullptr, QMediaPlayer::StreamPlayback);
    player->setAudioRole(QAudio::VideoRole);

    QObject::connect(
        player, &QMediaPlayer::stateChanged,
        [](QMediaPlayer::State state) {
            qDebug() << state;
        }
    );

    QObject::connect(
        player, &QMediaPlayer::audioAvailableChanged,
        [](bool available) {
            qDebug() << "Audio available : " + QString::number(available);
        }
    );

    QObject::connect(
        player, &QMediaPlayer::bufferStatusChanged,
        [](int percentFilled) {
            qDebug() << "Buffer filling : " + QString::number(percentFilled) + "%";
        }
    );

    QObject::connect(
        player, &QMediaPlayer::durationChanged,
        [](qint64 duration) {
            qDebug() << "Duration of current media : " + QString::number(duration / 1000)  + " seconds";
        }
    );

    QObject::connect(
        player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
        [](QMediaPlayer::Error error) {
            qDebug() << error;
        }
    );

    QObject::connect(
        player, &QMediaPlayer::mediaStatusChanged,
        [](QMediaPlayer::MediaStatus status) {
            qDebug() << status;
        }
    );

    QObject::connect(
        player, &QMediaPlayer::positionChanged,
        [](qint64 position) {
            // qDebug() << QString::number(position / 1000) + "s";
        }
    );

    QObject::connect(
        player, &QMediaPlayer::videoAvailableChanged,
        [](bool videoAvailable) {
            qDebug() << "video available : " + QString::number(videoAvailable);
        }
    );

    

    auto video = YoutubeVideoMetadata::fromUrl("https://www.youtube.com/watch?time_continue=211&v=6p03KD5IBrI");
    YoutubeHelper::refreshMetadata(video).then([=]() {
        
        auto test1 = QString("https://zonme.to2x.ovh/test.mp3");
        auto test2 = QString("https://zonme.to2x.ovh/test2.mp4");
        auto url = video->audioStreams()->getPreferedMineSourcePair().second;
        qDebug() << url;
        player->setMedia(QMediaContent(url));
        player->play();
    
    });

    //TestMainWindow test;
    return app.exec();
}

