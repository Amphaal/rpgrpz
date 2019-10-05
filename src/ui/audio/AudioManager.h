#pragma once

#include <QObject>
#include <QString>
#include <QMainWindow>

#include "src/_libs/promise.hpp"

#include "src/ui/audio/controllers/AudioProbeController.h"
#include "src/ui/audio/controllers/PlaylistController.h"

#include "src/shared/audio/GStreamerClient.h"

#include "src/network/youtube/YoutubeHelper.h"

#include "src/ui/_others/ClientBindable.h"

#include "src/shared/audio/StreamPlayStateTracker.hpp"

class AudioManager : public QWidget, public ClientBindable {
    
    Q_OBJECT
    
    public:
        AudioManager();

        PlaylistController* _plCtrl = nullptr;

    private slots:
        void _onIdentityAck(const RPZUser &user);
        void _onSeekingRequested(int seekPosInSecs);
        void _onSeekingRequested(qint64 seekPosInMsecs);
        void _onAudioPlayStateChanged(bool isPlaying);
        void _onAudioSourceStateChanged(const StreamPlayStateTracker &state);

    private:
        StreamPlayStateTracker _state;
        AudioProbeController* _asCtrl = nullptr;
        GStreamerClient* _cli = nullptr;


        bool _isLocalOnly = true;
        bool _isNetworkMaster = false;
        void onRPZClientConnecting() override;
        void onRPZClientDisconnect() override;

        void _link();

        void _stopPlayingMusic();
        void _playAudio(const QString &audioSourceUrl, const QString &sourceTitle, qint64 startAtMsecsPos);
        
        void _onToolbarActionRequested(const TrackToolbar::Action &action);
        void _onToolbarPlayRequested(YoutubeVideoMetadata* playlistItemPtr);
        void _onPlayerPositionChanged(int positionInSecs);
        void _onStreamPlayEnded();
        void _onStreamError();
    
};