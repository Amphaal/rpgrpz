#pragma once

#include <QGroupBox>
#include <QVBoxLayout>

#include "src/ui/others/ClientBindable.h"

#include "base/PlaylistToolbar.h"
#include "base/Playlist.h"

class PlaylistController : public QGroupBox {
    public:

        PlaylistController(QWidget * parent = nullptr);

        PlaylistToolbar* toolbar = nullptr;
        Playlist* playlist = nullptr;    

    private:
        void _onToolbarActionRequested(const PlaylistToolbar::Action &action);
};