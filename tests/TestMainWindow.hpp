#pragma once

#include <QMainWindow>

#include "src/ui/communication/audio/AudioStreamController.h"
#include "src/ui/communication/audio/PlaylistController.h"

#include "src/network/stream/GStreamerClient.h"

class TestMainWindow : public QMainWindow {
    public:
        TestMainWindow() : 
        _cli(new GStreamerClient(this)),
        _plCtrl(new PlaylistController),
        _asCtrl(new AudioStreamController) {
            
            //init
            _plCtrl->setEnabled(true);
            this->setCentralWidget(new QWidget);
            this->centralWidget()->setLayout(new QVBoxLayout);
            this->centralWidget()->layout()->addWidget(_plCtrl);
            this->centralWidget()->layout()->addWidget(_asCtrl);
            
            //connect
            QObject::connect(
                this->_plCtrl->playlist, &Playlist::playRequested,
                [&](const QString &uri) {
                    
                    this->_asCtrl->setEnabled(true);
                    this->_asCtrl->updatePlayedMusic(uri);

                    this->_plCtrl->toolbar->newTrack(0);
                    this->_cli->useSource(uri);
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
            this->showMaximized();
        }
    
    private:
        PlaylistController* _plCtrl = nullptr;
        AudioStreamController* _asCtrl = nullptr;
        GStreamerClient* _cli = nullptr;
};