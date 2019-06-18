#pragma once

#include <QGroupBox>
#include <QVBoxLayout>

#include "src/ui/others/ClientBindable.h"

#include "PlaylistToolbar.h"
#include "Playlist.h"

class PlaylistController : public QGroupBox, public ClientBindable {
    public:

        PlaylistController(QWidget * parent = nullptr);

        void onRPZClientConnecting(RPZClient * cc) override;
        void onRPZClientDisconnect(RPZClient* cc) override;

        PlaylistToolbar* toolbar = nullptr;
        Playlist* playlist = nullptr;    

    private:
        void _onToolbarActionRequested(const PlaylistToolbar::Action &action);
};