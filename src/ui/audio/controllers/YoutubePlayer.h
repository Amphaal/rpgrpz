#pragma once

#include <QGroupBox>
#include <QVBoxLayout>

#include "src/ui/_others/ConnectivityObserver.h"

#include "base/TrackToolbar.h"
#include "playlist/Playlist.h"
#include "playlist/YoutubePlaylistItemInsertor.hpp"

class YoutubePlayer : public QGroupBox {
    
    Q_OBJECT
    
    public:
        YoutubePlayer(QWidget * parent = nullptr);

        TrackToolbar* toolbar();
        Playlist* playlist();    

    private:
        TrackToolbar* _toolbar = nullptr;
        Playlist* _playlist = nullptr;    
        YoutubePlaylistItemInsertor* _linkInserter = nullptr;

        void _onToolbarActionRequested(const TrackToolbar::Action &action);
};