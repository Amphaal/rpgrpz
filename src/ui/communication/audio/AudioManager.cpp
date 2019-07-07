#include "AudioManager.h"

AudioManager::AudioManager() : 
    _cli(new GStreamerClient),   
    _plCtrl(new PlaylistController), 
    _asCtrl(new AudioStreamController) {

    //UI init
    this->_plCtrl->setEnabled(true);
    this->setLayout(new QVBoxLayout);
    this->setContentsMargins(0, 0, 0, 0);
    this->layout()->addWidget(_plCtrl);
    this->layout()->addWidget(_asCtrl);

    //link between inner elements
    this->_link();
}

void AudioManager::onRPZClientConnecting(RPZClient * cc) {
    ClientBindable::onRPZClientConnecting(cc);
    
    //reset state
    this->_isLocalOnly = false;
    this->_isNetworkMaster = false;
    this->_plCtrl->setEnabled(false);
    this->_onStreamPlayEnded();

    //on receiving identity
    QObject::connect(
        cc, &RPZClient::ackIdentity,
        [&]() {
            this->_isNetworkMaster = this->_rpzClient->identity().role() == RPZUser::Role::Host;
            this->_plCtrl->setEnabled(this->_isNetworkMaster);
        }
    );

    //on master requesting audio change
    QObject::connect(
        cc, &RPZClient::audioSourceChanged,
        this, &AudioManager::_playAudio
    );

    //on master seeking
    QObject::connect(
        cc, &RPZClient::audioPositionChanged,
        this, &AudioManager::_onSeekingRequested
    );

    //on master pausing / playing
    QObject::connect(
        cc, &RPZClient::audioPlayStateChanged,
        [&](bool isPlaying) {
            if(isPlaying) this->_cli->play();
            else this->_cli->pause();
        }
    );

}

void AudioManager::onRPZClientDisconnect(RPZClient* cc) {
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

    //on action required from toolbar
    QObject::connect(
        this->_plCtrl->toolbar, &PlaylistToolbar::actionRequired,
        this, &AudioManager::_onToolbarActionRequested
    );

    QObject::connect(
        this->_plCtrl->toolbar, &PlaylistToolbar::seeking,
        this, &AudioManager::_onSeekingRequested
    );

    //volume change from toolbar
    QObject::connect(
        this->_asCtrl->toolbar, &AudioStreamToolbar::askForVolumeChange,
        this->_cli, &GStreamerClient::setVolume
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

void AudioManager::_onToolbarActionRequested(const PlaylistToolbar::Action &action) {
    switch(action) {
        case PlaylistToolbar::Action::Play:
            this->_cli->play();
        break;
        case PlaylistToolbar::Action::Pause:
            this->_cli->pause();
        break;
        default:
            break;
    }
}

void AudioManager::_onToolbarPlayRequested(void* playlistItemPtr) {

    auto playlistItem = (PlaylistItem*)playlistItemPtr;

    playlistItem->streamSourceUri().then([=](const QString &sourceUrlStr) {
        
        auto title = playlistItem->title();
        this->_playAudio(sourceUrlStr, title);

        //play new track
        this->_plCtrl->toolbar->newTrack(playlistItem->durationSecs());


        //tells others users what to listen to
        if(this->_isNetworkMaster) {
            this->_rpzClient->defineAudioStreamSource(sourceUrlStr, title);
        }
    });

}

void AudioManager::_onStreamPlayEnded() {
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
    if(this->_isNetworkMaster || this->_isLocalOnly) {
        this->_cli->seek(seekPos);
    }
}