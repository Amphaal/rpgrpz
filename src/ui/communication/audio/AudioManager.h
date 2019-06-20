#pragma once

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaContent>
#include <QMediaResource>

#include "src/_libs/qtPromise/qpromise.h"

#include "src/ui/communication/audio/controllers/AudioStreamController.h"
#include "src/ui/communication/audio/controllers/PlaylistController.h"


#include "src/helpers/network/youtube/YoutubeHelper.h"

class AudioManager : public QWidget {
    public:
        AudioManager();
    
    private:
        PlaylistController* _plCtrl = nullptr;
        AudioStreamController* _asCtrl = nullptr;
        QMediaPlayer* _cli = nullptr;
};