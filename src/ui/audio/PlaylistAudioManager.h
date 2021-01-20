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

#pragma once

#include <QObject>
#include <QString>
#include <QMainWindow>

#include <audiotube/NetworkFetcher.h>

#include "src/ui/audio/controllers/AudioProbeController.h"
#include "src/ui/audio/controllers/YoutubePlayer.h"

#include "src/shared/audio/GStreamerClient.h"

#include "src/ui/_others/ConnectivityObserver.h"

#include "src/shared/audio/StreamPlayStateTracker.hpp"

class PlaylistAudioManager : public QWidget, public ConnectivityObserver {
    Q_OBJECT

 public:
    explicit PlaylistAudioManager(QWidget *parent = nullptr);

    YoutubePlayer* player();

 private:
    void _onGameSessionReceived(const RPZGameSession &gameSession);
    void _onSeekingRequested(int seekPosInSecs);
    void _onSeekingRequested(qint64 seekPosInMsecs);
    void _onAudioPlayStateChanged(bool isPlaying);
    void _onAudioSourceStateChanged(const StreamPlayStateTracker &state);

    AudioProbeController* _asCtrl = nullptr;
    YoutubePlayer* _plCtrl = nullptr;

    StreamPlayStateTracker _state;
    GStreamerClient* _cli = nullptr;

    bool _isLocalOnly = true;
    bool _isNetworkMaster = false;
    void connectingToServer() override;
    void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override;

    void _link();

    void _stopPlayingMusic();
    void _playAudio(const QString &audioSourceUrl, const QString &sourceTitle, qint64 startAtMsecsPos);
    void _applyPlayRequestFromState();

    void _onToolbarActionRequested(const TrackToolbar::Action &action);
    void _onPlayRequested(AudioTube::VideoMetadata* playlistItemPtr);
    void _onPlayerPositionChanged(int positionInSecs);
    void _onStreamPlayEnded();
    void _onStreamError();
};
