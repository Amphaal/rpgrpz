#include "YoutubePlayer.h"

YoutubePlayer::YoutubePlayer(QWidget * parent) : QGroupBox(tr("Playlist"), parent), 
    _playlist(new Playlist(this)), 
    _toolbar(new TrackToolbar(this)),
    _linkInserter(new YoutubePlaylistItemInsertor(this)) {

    //self
    this->setEnabled(false);
    this->setLayout(new QVBoxLayout);

    //groupbox
    this->setAlignment(Qt::AlignHCenter);

    //on action from toolbar
    QObject::connect(
        this->_toolbar, &TrackToolbar::actionRequired,
        this, &YoutubePlayer::_onToolbarActionRequested
    );

    //on youtube url insertion
    QObject::connect(
        this->_linkInserter, &YoutubePlaylistItemInsertor::insertionRequested,
        this->_playlist, &Playlist::addYoutubeVideo
    );

    //inner list
    this->layout()->addWidget(this->_toolbar);
    this->layout()->addWidget(this->_linkInserter);
    this->layout()->addWidget(this->_playlist);
};

TrackToolbar* YoutubePlayer::toolbar() {
    return this->_toolbar;
}

Playlist* YoutubePlayer::playlist() {
    return this->_playlist;
}

void YoutubePlayer::_onToolbarActionRequested(const TrackToolbar::Action &action) {
    switch(action) {

        case TrackToolbar::Action::Forward:
            this->_playlist->playNext();
            break;
            
        case TrackToolbar::Action::Rewind:
            this->_playlist->playPrevious();
            break;

        default:
            break;

    }
}
