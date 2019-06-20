#include "AudioManager.h"

AudioManager::AudioManager() : 
    _cli(new QMediaPlayer(this)), 
    _plCtrl(new PlaylistController), 
    _asCtrl(new AudioStreamController) {
    
    //init
    this->_plCtrl->setEnabled(true);
    this->setLayout(new QVBoxLayout);
    this->setContentsMargins(0, 0, 0, 0);
    this->layout()->addWidget(_plCtrl);
    this->layout()->addWidget(_asCtrl);
    
    //connect
    QObject::connect(
        this->_plCtrl->playlist, &Playlist::playRequested,
        [&](void* playlistItemPtr) {
            auto playlistItem = (PlaylistItem*)playlistItemPtr;

            this->_asCtrl->setEnabled(true);
            this->_asCtrl->updatePlayedMusic(playlistItem->title());

            this->_plCtrl->toolbar->newTrack(0);
            playlistItem->streamSourceUri().then([=](const QString &sourceUrlStr) {
                QUrl url(sourceUrlStr);
                QMediaContent content(url);
                this->_cli->setMedia(content);
                this->_cli->setVolume(50);
                this->_cli->play();
            });
        }
    );

    //on reading error
    QObject::connect(
        this->_cli, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
        [&](QMediaPlayer::Error error) {
            qDebug() << error;
        }
    );

    //on status change
    QObject::connect(
        this->_cli, &QMediaPlayer::mediaStatusChanged,
        [&](QMediaPlayer::MediaStatus status) {
            qDebug() << status;
        }
    );

    QObject::connect(
        this->_plCtrl->toolbar, &PlaylistToolbar::actionRequired,
        [&](const PlaylistToolbar::Action &action) {
            switch(action) {
                case PlaylistToolbar::Action::Play:
                    this->_cli->play();
                break;
                case PlaylistToolbar::Action::Pause:
                    this->_cli->pause();
                break;
            }
        }
    );

    //volume change
    QObject::connect(
        this->_asCtrl->toolbar, &AudioStreamToolbar::askForVolumeChange,
        this->_cli, &QMediaPlayer::setVolume
    );

}
