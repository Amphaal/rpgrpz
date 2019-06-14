#include "TestMainWindow.h"

TestMainWindow::TestMainWindow() : _cli(new GStreamerClient(this)), _plCtrl(new PlaylistController), _asCtrl(new AudioStreamController) {
    
    //init
    this->_plCtrl->setEnabled(true);
    this->setCentralWidget(new QWidget);
    this->centralWidget()->setLayout(new QVBoxLayout);
    this->centralWidget()->layout()->addWidget(_plCtrl);
    this->centralWidget()->layout()->addWidget(_asCtrl);
    
    //connect
    QObject::connect(
        this->_plCtrl->playlist, &Playlist::playRequested,
        [&](void* item) {
            auto c_pointer = (PlaylistItem*)item;

            this->_asCtrl->setEnabled(true);
            this->_asCtrl->updatePlayedMusic(c_pointer->title());

            this->_plCtrl->toolbar->newTrack(0);
            c_pointer->streamSourceUri().then([=](const QString &source) {
                this->_cli->useSource(source);
            });
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

    QObject::connect(
        this->_asCtrl->toolbar, &AudioStreamToolbar::askForVolumeChange,
        this->_cli, &GStreamerClient::setVolume
    );


    //initial show
    this->resize(800, 600);
    this->show();
}
