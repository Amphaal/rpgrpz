#pragma once

#include <QMainWindow>

#include "src/_libs/qtPromise/qpromise.h"

#include "src/ui/communication/audio/AudioStreamController.h"
#include "src/ui/communication/audio/PlaylistController.h"

#include "src/network/stream/GStreamerClient.h"
#include "src/helpers/network/youtube/YoutubeHelper.h"

class TestMainWindow : public QMainWindow {
    public:
        TestMainWindow();
    
    private:
        PlaylistController* _plCtrl = nullptr;
        AudioStreamController* _asCtrl = nullptr;
        GStreamerClient* _cli = nullptr;
};