#pragma once

#include <QMainWindow>

#include "src/_libs/qtPromise/qpromise.h"

#include "src/ui/communication/audio/controllers/AudioStreamController.h"
#include "src/ui/communication/audio/controllers/PlaylistController.h"

#include <QMediaPlayer>

#include "src/helpers/network/youtube/YoutubeHelper.h"

class AudioManager : public QWidget {
    public:
        AudioManager();
    
    private:
        PlaylistController* _plCtrl = nullptr;
        AudioStreamController* _asCtrl = nullptr;
        QMediaPlayer* _cli = nullptr;

        void _link();
        void _onToolbarActionRequested(const PlaylistToolbar::Action &action);
        void _onToolbarPlayRequested(void* playlistItemPtr);
        void _onPlayerPositionChanged(int position);
};