// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "YoutubePlayer.h"

YoutubePlayer::YoutubePlayer(QWidget * parent) : QGroupBox(tr("Playlist"), parent), _toolbar(new TrackToolbar(this)),
    _playlist(new Playlist(this)), _linkInserter(new YoutubePlaylistItemInsertor(this)) {
    // self
    this->setEnabled(false);
    this->setLayout(new QVBoxLayout);

    // groupbox
    this->setAlignment(Qt::AlignHCenter);

    // on action from toolbar
    QObject::connect(
        this->_toolbar, &TrackToolbar::actionRequired,
        this, &YoutubePlayer::_onToolbarActionRequested
    );

    // on youtube url insertion
    QObject::connect(
        this->_linkInserter, &YoutubePlaylistItemInsertor::insertionRequested,
        this->_playlist, &Playlist::addYoutubeVideo
    );

    // inner list
    this->layout()->addWidget(this->_toolbar);
    this->layout()->addWidget(this->_linkInserter);
    this->layout()->addWidget(this->_playlist);
};

TrackToolbar* YoutubePlayer::toolbar() {
    return this->_toolbar;
}

Playlist* YoutubePlayer::playlist() {
    return this->_playlist;
}

void YoutubePlayer::_onToolbarActionRequested(const TrackToolbar::Action &action) {
    switch (action) {
        case TrackToolbar::Action::Forward:
            this->_playlist->playNext();
            break;

        case TrackToolbar::Action::Rewind:
            this->_playlist->playPrevious();
            break;

        default:
            break;
    }
}
