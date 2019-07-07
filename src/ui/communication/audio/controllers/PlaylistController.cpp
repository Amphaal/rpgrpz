#include "PlaylistController.h"

PlaylistController::PlaylistController(QWidget * parent) : QGroupBox("Liste de lecture", parent), 
    playlist(new Playlist(this)), 
    toolbar(new PlaylistToolbar(this)) {

    //self
    this->setEnabled(false);
    this->setLayout(new QVBoxLayout);

    //groupbox
    this->setAlignment(Qt::AlignHCenter);

    //connect
    QObject::connect(
        this->toolbar, &PlaylistToolbar::actionRequired,
        this, &PlaylistController::_onToolbarActionRequested
    );

    //inner list
    this->layout()->addWidget(this->toolbar);
    this->layout()->addWidget(this->playlist);
};

void PlaylistController::_onToolbarActionRequested(const PlaylistToolbar::Action &action) {
    switch(action) {
        case PlaylistToolbar::Action::Pause:
            break;
        case PlaylistToolbar::Action::Play:
            break;
        case PlaylistToolbar::Action::Forward:
            this->playlist->playNext();
            break;
        case PlaylistToolbar::Action::Rewind:
            this->playlist->playPrevious();
            break;
        default:
            break;
    }
}
