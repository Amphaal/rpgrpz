// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

#include <QGroupBox>
#include <QVBoxLayout>

#include "src/ui/audio/controllers/_base/TrackToolbar.h"
#include "src/ui/audio/controllers/playlist/Playlist.h"
#include "src/ui/audio/controllers/playlist/YoutubePlaylistItemInsertor.hpp"

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