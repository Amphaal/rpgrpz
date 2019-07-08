#include "PlaylistController.h"

PlaylistController::PlaylistController(QWidget * parent) : QGroupBox("Liste de lecture", parent), 
    playlist(new Playlist(this)), 
    toolbar(new TrackToolbar(this)) {

    //self
    this->setEnabled(false);
    this->setLayout(new QVBoxLayout);

    //groupbox
    this->setAlignment(Qt::AlignHCenter);

    //connect
    QObject::connect(
        this->toolbar, &TrackToolbar::actionRequired,
        this, &PlaylistController::_onToolbarActionRequested
    );

    //inner list
    this->layout()->addWidget(this->toolbar);
    this->layout()->addWidget(this->playlist);
};

void PlaylistController::_onToolbarActionRequested(const TrackToolbar::Action &action) {
    switch(action) {
        case TrackToolbar::Action::Pause:
            break;
        case TrackToolbar::Action::Play:
            break;
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
