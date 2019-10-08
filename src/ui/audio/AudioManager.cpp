#include "AudioManager.h"

AudioManager::AudioManager() : 
    _cli(new GStreamerClient),   
    _plCtrl(new PlaylistController), 
    _asCtrl(new AudioProbeController) {

    //UI init
    this->_plCtrl->setEnabled(true);
    this->setLayout(new QVBoxLayout);
    this->setContentsMargins(0, 0, 0, 0);
    this->layout()->addWidget(_plCtrl);
    this->layout()->addWidget(_asCtrl);

    //link between inner elements
    this->_link();
}

void AudioManager::_onIdentityAck(const RPZUser &user) {
    
    this->_isNetworkMaster = user.role() == RPZUser::Role::Host;
    this->_plCtrl->setEnabled(this->_isNetworkMaster);
    
    if(!this->_isNetworkMaster) {

        //stop playing music and wait for online instructions
        this->_stopPlayingMusic();

    } else {

        //send state to client
        QMetaObject::invokeMethod(this->_rpzClient, "defineAudioSourceState", 
            Q_ARG(StreamPlayStateTracker, this->_state)
        );

    }
    
}

void AudioManager::onRPZClientConnecting() {
    
    //reset state
    this->_isLocalOnly = false;
    this->_isNetworkMaster = false;
    this->_plCtrl->setEnabled(false);

    //on receiving identity
    QObject::connect(
        _rpzClient, &RPZClient::ackIdentity,
        this, &AudioManager::_onIdentityAck
    );

    //on master requesting audio change
    QObject::connect(
        _rpzClient, &RPZClient::audioSourceStateChanged,
        this, &AudioManager::_onAudioSourceStateChanged
    );

    //on master seeking
    QObject::connect(
        _rpzClient, &RPZClient::audioPositionChanged,
        this, qOverload<qint64>(&AudioManager::_onSeekingRequested)
    );

    //on master pausing / playing
    QObject::connect(
        _rpzClient, &RPZClient::audioPlayStateChanged,
        this, &AudioManager::_onAudioPlayStateChanged
    );

}

void AudioManager::_onAudioSourceStateChanged(const StreamPlayStateTracker &state) {

    //update state
    this->_state = state;

    //play audio
    this->_playAudio(
        this->_state.url(), 
        this->_state.title(), 
        this->_state.positionInMsecs()
    );

}

void AudioManager::_onAudioPlayStateChanged(bool isPlaying) {
    
    //update state
    this->_state.updatePlayingState(isPlaying);
    
    //use audio cli
    if(isPlaying) this->_cli->play();
    else this->_cli->pause();
}

void AudioManager::onRPZClientDisconnect() {
    
    if(!this->_isNetworkMaster) {
        this->_stopPlayingMusic();
    }
    
    this->_isNetworkMaster = false;
    this->_isLocalOnly = true;
    this->_plCtrl->setEnabled(true);

}


void AudioManager::_link() {
    
    //on play requested from playlist
    QObject::connect(
        this->_plCtrl->playlist, &Playlist::playRequested,
        this, &AudioManager::_onToolbarPlayRequested
    );

        //on action required from toolbar
    QObject::connect(
        this->_plCtrl->toolbar, &TrackToolbar::actionRequired,
        this, &AudioManager::_onToolbarActionRequested
    );

    //on seeking from toolbar
    QObject::connect(
        this->_plCtrl->toolbar, &TrackToolbar::seeking,
        this, qOverload<int>(&AudioManager::_onSeekingRequested)
    );

    //volume change from toolbar
    QObject::connect(
        this->_asCtrl->toolbar, &VolumeToolbar::askForVolumeChange,
        this->_cli, &GStreamerClient::setVolume
    );

    //player position changed
    QObject::connect(
        this->_cli, &GStreamerClient::positionChanged,
        this, &AudioManager::_onPlayerPositionChanged
    );

    //on stream play ended
    QObject::connect(
        this->_cli, &GStreamerClient::streamEnded,
        this, &AudioManager::_onStreamPlayEnded
    );

    //on stream error while trying to play it
    QObject::connect(
        this->_cli, &GStreamerClient::streamError,
        this, &AudioManager::_onStreamError
    );

    //on cli play state changed
    QObject::connect(
        this->_cli, &GStreamerClient::playStateChanged,
        this->_asCtrl, &AudioProbeController::changeTrackState
    );

}

void AudioManager::_playAudio(const QString &audioSourceUrl, const QString &sourceTitle, qint64 startAtMsecsPos) {
    
    //if start is not > -1, should not play because stream ended!
    if(startAtMsecsPos == -1) return;

    //update state
    this->_state.updatePositionInMSecs(startAtMsecsPos);
    
    //update ui
    this->_asCtrl->updatePlayedMusic(sourceTitle);
    
    this->_cli->stop(); //stop
    this->_cli->useSource(audioSourceUrl); //use source
    this->_cli->play(); //play
    if(startAtMsecsPos > 0) {
        this->_cli->seek(startAtMsecsPos); //seek to pos if requested
    }

}

//
// events helpers
//

void AudioManager::_onToolbarActionRequested(const TrackToolbar::Action &action) {
    
    switch(action) {
        
        case TrackToolbar::Action::Play: {
            
            //play audio cli
            this->_cli->play();
            
            //update state
            this->_state.updatePlayingState(true);

            //send to network
            if(this->_isNetworkMaster) {
                QMetaObject::invokeMethod(this->_rpzClient, "setAudioStreamPlayState", 
                    Q_ARG(bool, true)
                );
            }

        }
        break;

        case TrackToolbar::Action::Pause: {
            
            //pause audio cli
            this->_cli->pause();

            //update state
            this->_state.updatePlayingState(false);

            //send to network
            if(this->_isNetworkMaster) {
                QMetaObject::invokeMethod(this->_rpzClient, "setAudioStreamPlayState", 
                    Q_ARG(bool, false)
                );
            }

        }
        break;

        default:
            break;

    }


}

void AudioManager::_onToolbarPlayRequested(YoutubeVideoMetadata* metadata) {

    YoutubeHelper::refreshMetadata(metadata).then([=]() {
        
        auto title = metadata->title();
        auto streamUrl = metadata->audioStreams()->getPreferedMineSourcePair().second;
        auto duration = metadata->duration();

        //update state
        this->_state.registerNewPlay(streamUrl, title, duration);

        //update UI
        this->_plCtrl->toolbar->newTrack(duration);

        //play audio cli
        this->_playAudio(streamUrl, title, 0);

        //tells others users what to listen to
        if(this->_isNetworkMaster) {

            //send to client
            QMetaObject::invokeMethod(this->_rpzClient, "defineAudioSourceState", 
                Q_ARG(StreamPlayStateTracker, this->_state)
            );

        }

    });

}

void AudioManager::_onStreamError() {
   
    //reset state
    this->_state.clear();

    //update UI
    this->_plCtrl->toolbar->endTrack();
    this->_asCtrl->updatePlayedMusic(NULL);

    //tells that current play failed
    auto currentPlay = this->_plCtrl->playlist->currentPlay();
    currentPlay->setFailure(true);
}

void AudioManager::_stopPlayingMusic() {
    
    //reset state
    this->_state.clear();
    
    //stop audio
    this->_cli->stop();

    //update UI
    this->_plCtrl->toolbar->endTrack();
    this->_asCtrl->updatePlayedMusic(NULL);

}

void AudioManager::_onStreamPlayEnded() {
    
    this->_stopPlayingMusic();
    
    //auto play
    if(this->_isNetworkMaster || this->_isLocalOnly) {
        this->_plCtrl->playlist->playNext();
    }

}

void AudioManager::_onPlayerPositionChanged(int positionInSecs) {
    
    //update UI
    if(this->_isNetworkMaster || this->_isLocalOnly) {
        this->_plCtrl->toolbar->updatePlayerPosition(positionInSecs);
    }

    this->_asCtrl->changeTrackPosition(positionInSecs);

}

void AudioManager::_onSeekingRequested(qint64 seekPosInMsecs) {
    
    //seek audio client
    this->_cli->seek(seekPosInMsecs);
    
    //update state
    this->_state.updatePositionInMSecs(seekPosInMsecs);

    //may advertise client
    if(this->_isNetworkMaster) {
        QMetaObject::invokeMethod(this->_rpzClient, "changeAudioPosition", 
            Q_ARG(qint64, seekPosInMsecs)
        );
    }

}

void AudioManager::_onSeekingRequested(int seekPosInSecs) {
    this->_onSeekingRequested((qint64)seekPosInSecs * 1000);
}