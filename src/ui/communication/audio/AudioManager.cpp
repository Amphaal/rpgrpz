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

    //on action required from toolbar
    QObject::connect(
        this->_plCtrl->toolbar, &PlaylistToolbar::actionRequired,
        this, &AudioManager::_onToolbarActionRequested
    );

    //volume change from toolbar
    QObject::connect(
        this->_asCtrl->toolbar, &AudioStreamToolbar::askForVolumeChange,
        this->_cli, &GStreamerClient::setVolume
    );

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
        
        this->_asCtrl->setEnabled(true);
        this->_asCtrl->updatePlayedMusic(playlistItem->title());

        this->_plCtrl->toolbar->newTrack(playlistItem->durationSecs());
        this->_cli->useSource(sourceUrlStr);
        this->_cli->play();
    });

}

void AudioManager::_onPlayerPositionChanged(int position) {
    this->_plCtrl->toolbar->updateTrackState(position);
}