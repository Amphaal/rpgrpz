#pragma once

#include <QGroupBox>
#include <QVBoxLayout>

#include "src/ui/_others/ClientBindable.h"

#include "base/TrackToolbar.h"
#include "playlist/Playlist.h"
#include "playlist/YoutubePlaylistItemInsertor.hpp"

class PlaylistController : public QGroupBox {
    
    Q_OBJECT
    
    public:
        PlaylistController(QWidget * parent = nullptr);

        TrackToolbar* toolbar = nullptr;
        Playlist* playlist = nullptr;    
        YoutubePlaylistItemInsertor* linkInserter = nullptr;

    private:
        void _onToolbarActionRequested(const TrackToolbar::Action &action);
};