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

void AudioManager::onRPZClientThreadConnecting(RPZClientThread * cc) {
    ClientBindable::onRPZClientThreadConnecting(cc);
    
    //reset state
    this->_isLocalOnly = false;
    this->_isNetworkMaster = false;
    this->_plCtrl->setEnabled(false);
    this->_onStreamPlayEnded();

    //on receiving identity
    QObject::connect(
        cc, &RPZClientThread::ackIdentity,
        [&]() {
            this->_isNetworkMaster = this->_rpzClient->identity().role() == RPZUser::Role::Host;
            this->_plCtrl->setEnabled(this->_isNetworkMaster);
        }
    );

    //on master requesting audio change
    QObject::connect(
        cc, &RPZClientThread::audioSourceChanged,
        this, &AudioManager::_playAudio
    );

    //on master seeking
    QObject::connect(
        cc, &RPZClientThread::audioPositionChanged,
        this, &AudioManager::_onSeekingRequested
    );

    //on master pausing / playing
    QObject::connect(
        cc, &RPZClientThread::audioPlayStateChanged,
        [&](bool isPlaying) {
            if(isPlaying) this->_cli->play();
            else this->_cli->pause();
        }
    );

}

void AudioManager::onRPZClientThreadDisconnect(RPZClientThread* cc) {
    this->_isNetworkMaster = false;
    this->_isLocalOnly = true;
    this->_plCtrl->setEnabled(true);
    this->_onStreamPlayEnded();
}


void AudioManager::_link() {
    
    //on play requested from playlist
    QObject::connect(
        this->_plCtrl->playlist, &Playlist::playRequested,
        this, &AudioManager::_onToolbarPlayRequested
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

    //on action required from toolbar
    QObject::connect(
        this->_plCtrl->toolbar, &TrackToolbar::actionRequired,
        this, &AudioManager::_onToolbarActionRequested
    );

    //on seeking from toolbar
    QObject::connect(
        this->_plCtrl->toolbar, &TrackToolbar::seeking,
        this, &AudioManager::_onSeekingRequested
    );

    //volume change from toolbar
    QObject::connect(
        this->_asCtrl->toolbar, &VolumeToolbar::askForVolumeChange,
        this->_cli, &GStreamerClient::setVolume
    );

    //on cli play state changed
    QObject::connect(
        this->_cli, &GStreamerClient::playStateChanged,
        [&](bool isPlaying) {
            this->_asCtrl->changeTrackState(isPlaying);
        }
    );

}

void AudioManager::_playAudio(const QString &audioSourceUrl, const QString &sourceTitle) {
    this->_asCtrl->updatePlayedMusic(sourceTitle);
    this->_cli->useSource(audioSourceUrl);
    this->_cli->play();
}

//
// events helpers
//

void AudioManager::_onToolbarActionRequested(const TrackToolbar::Action &action) {
    
    switch(action) {
        
        case TrackToolbar::Action::Play:
            this->_cli->play();
            if(this->_isNetworkMaster) this->_rpzClient->setAudioStreamPlayState(true);
        break;

        case TrackToolbar::Action::Pause:
            this->_cli->pause();
            if(this->_isNetworkMaster) this->_rpzClient->setAudioStreamPlayState(false);
        break;

        default:
            break;

    }


}

void AudioManager::_onToolbarPlayRequested(YoutubeVideoMetadata* metadata) {

    YoutubeHelper::refreshMetadata(metadata).then([=]() {
        
        auto title = metadata->title();
        auto streamUrl = metadata->audioStreams()->getPreferedMineSourcePair().second;
        
        this->_playAudio(streamUrl, title);

        //play new track
        this->_plCtrl->toolbar->newTrack(metadata->duration());

        //tells others users what to listen to
        if(this->_isNetworkMaster) {
            this->_rpzClient->defineAudioStreamSource(streamUrl, title);
        }
    });

}

void AudioManager::_onStreamError() {
    this->_plCtrl->toolbar->endTrack();
    this->_asCtrl->updatePlayedMusic(NULL);

    auto currentPlay = this->_plCtrl->playlist->currentPlay();
    currentPlay->setFailure(true);
}

void AudioManager::_onStreamPlayEnded() {
    this->_cli->stop();
    this->_plCtrl->toolbar->endTrack();
    this->_asCtrl->updatePlayedMusic(NULL);

    if(this->_isNetworkMaster || this->_isLocalOnly) {
        this->_plCtrl->playlist->playNext();
    }
}

void AudioManager::_onPlayerPositionChanged(int position) {
    if(this->_isNetworkMaster || this->_isLocalOnly) {
        this->_plCtrl->toolbar->updateTrackState(position);
    }
}

void AudioManager::_onSeekingRequested(int seekPos) {
    this->_cli->seek(seekPos);
    if(this->_isNetworkMaster) {
        this->_rpzClient->changeAudioPosition(seekPos);
    }
}