#pragma once

#include <QObject>
#include <QString>
#include <QMainWindow>

#include "src/_libs/promise.hpp"

#include "src/ui/communication/audio/controllers/AudioProbeController.h"
#include "src/ui/communication/audio/controllers/PlaylistController.h"

#include "src/shared/audio/GStreamerClient.h"

#include "src/network/youtube/YoutubeHelper.h"

#include "src/ui/others/ClientBindable.h"

class AudioManager : public QWidget, public ClientBindable {
    
    Q_OBJECT
    
    public:
        AudioManager();

        PlaylistController* _plCtrl = nullptr;

    private slots:
        void _onIdentityAck(const RPZUser &user);
        void _onAudioPlayStateChanged(bool isPlaying);
        void _onSeekingRequested(int seekPos);
        void _playAudio(const QString &audioSourceUrl, const QString &sourceTitle, int startAt);

    private:
        AudioProbeController* _asCtrl = nullptr;
        GStreamerClient* _cli = nullptr;

        bool _isLocalOnly = true;
        bool _isNetworkMaster = false;
        void onRPZClientConnecting() override;
        void onRPZClientDisconnect() override;

        void _link();
        void _onToolbarActionRequested(const TrackToolbar::Action &action);
        void _onToolbarPlayRequested(YoutubeVideoMetadata* playlistItemPtr);
        void _onPlayerPositionChanged(int position);
        
        void _onStreamPlayEnded();
        void _onStreamError();
        
};