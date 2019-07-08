#pragma once

#include <QGroupBox>
#include <QVBoxLayout>

#include "src/ui/others/ClientBindable.h"

#include "base/TrackToolbar.h"
#include "playlist/Playlist.h"

class PlaylistController : public QGroupBox {
    public:

        PlaylistController(QWidget * parent = nullptr);

        TrackToolbar* toolbar = nullptr;
        Playlist* playlist = nullptr;    

    private:
        void _onToolbarActionRequested(const TrackToolbar::Action &action);
};