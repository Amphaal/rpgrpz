#include "PlaylistController.h"

PlaylistController::PlaylistController(QWidget * parent) : QGroupBox("Liste de lecture", parent), 
    playlist(new Playlist(this)), 
    toolbar(new TrackToolbar(this)),
    linkInserter(new YoutubePlaylistItemInsertor(this)) {

    //self
    this->setEnabled(false);
    this->setLayout(new QVBoxLayout);

    //groupbox
    this->setAlignment(Qt::AlignHCenter);

    //on action from toolbar
    QObject::connect(
        this->toolbar, &TrackToolbar::actionRequired,
        this, &PlaylistController::_onToolbarActionRequested
    );

    //on youtube url insertion
    QObject::connect(
        this->linkInserter, &YoutubePlaylistItemInsertor::insertionRequested,
        this->playlist, &Playlist::addYoutubeVideo
    );

    //inner list
    this->layout()->addWidget(this->toolbar);
    this->layout()->addWidget(this->linkInserter);
    this->layout()->addWidget(this->playlist);
};

void PlaylistController::_onToolbarActionRequested(const TrackToolbar::Action &action) {
    switch(action) {

        case TrackToolbar::Action::Forward:
            this->playlist->playNext();
            break;
            
        case TrackToolbar::Action::Rewind:
            this->playlist->playPrevious();
            break;

        default:
            break;

    }
}
