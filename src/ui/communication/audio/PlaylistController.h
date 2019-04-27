#pragma once

#include <QGroupBox>
#include <QVBoxLayout>

#include "src/ui/others/ClientBindable.hpp"

#include "PlaylistToolbar.hpp"
#include "Playlist.h"

class PlaylistController : public QGroupBox, public ClientBindable {
    public:

        PlaylistController(QWidget * parent = nullptr);

        void bindToRPZClient(RPZClient * cc) override;

        PlaylistToolbar* toolbar = nullptr;
        Playlist* playlist = nullptr;    

    private:
        void _onToolbarActionRequested(const PlaylistToolbar::Action &action);
};