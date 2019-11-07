#pragma once

#include <QObject>
#include <QString>
#include <QMainWindow>

#include "src/_libs/promise.hpp"

#include "src/ui/audio/controllers/AudioProbeController.h"
#include "src/ui/audio/controllers/YoutubePlayer.h"

#include "src/shared/audio/GStreamerClient.h"

#include "src/network/youtube/YoutubeHelper.h"

#include "src/ui/_others/ConnectivityObserver.h"

#include "src/shared/audio/StreamPlayStateTracker.hpp"

class AudioManager : public QWidget, public ConnectivityObserver {
    
    Q_OBJECT
    
    public:
        AudioManager(QWidget *parent = nullptr);

        YoutubePlayer* player();

    private slots:
        void _onIdentityAck(const RPZUser &user);
        void _onSeekingRequested(int seekPosInSecs);
        void _onSeekingRequested(qint64 seekPosInMsecs);
        void _onAudioPlayStateChanged(bool isPlaying);
        void _onAudioSourceStateChanged(const StreamPlayStateTracker &state);

    private:
        AudioProbeController* _asCtrl = nullptr;
        YoutubePlayer* _plCtrl = nullptr;

        StreamPlayStateTracker _state;
        GStreamerClient* _cli = nullptr;


        bool _isLocalOnly = true;
        bool _isNetworkMaster = false;
        void connectingToServer() override;
        void connectionClosed(bool hasInitialMapLoaded) override;

        void _link();

        void _stopPlayingMusic();
        void _playAudio(const QString &audioSourceUrl, const QString &sourceTitle, qint64 startAtMsecsPos);
        
        void _onToolbarActionRequested(const TrackToolbar::Action &action);
        void _onToolbarPlayRequested(YoutubeVideoMetadata* playlistItemPtr);
        void _onPlayerPositionChanged(int positionInSecs);
        void _onStreamPlayEnded();
        void _onStreamError();
    
};