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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QTime>
#include <QString>

#include <QWidget>
#include <QHBoxLayout>

#include <QLabel>
#include <QSlider>
#include <QToolButton>

#include "src/helpers/StringHelper.hpp"

class TrackToolbar : public QWidget {
    Q_OBJECT

 public:
    enum class Action { Rewind, Forward, Play, Pause };
    enum class RepeatBehavior { RepeatAll, RepeatOne };
    Q_ENUM(Action)

    explicit TrackToolbar(QWidget* parent = nullptr);

    void updatePlayerPosition(int posInSeconds);
    void newTrack(int lengthInSeconds);
    void endTrack();

    TrackToolbar::RepeatBehavior repeatBehavior() const;

 signals:
        void actionRequired(const TrackToolbar::Action &action);
        void seeking(int posInSecs);

 private:
    static inline const QString _defaultNoTime = "--";
    static inline const QString _trackPlayStateTemplator = " %1 / %2 ";
    static inline TrackToolbar::RepeatBehavior _repeatBehavior = RepeatBehavior::RepeatAll;

    QIcon _playIcon = QIcon(QStringLiteral(u":/icons/app/audio/play.png"));
    QIcon _pauseIcon = QIcon(QStringLiteral(u":/icons/app/audio/pause.png"));

    QToolButton* _playBtn = nullptr;
    QToolButton* _replayBtn = nullptr;
    QToolButton* _rewindBtn = nullptr;
    QToolButton* _forwardBtn = nullptr;

    QSlider* _trackStateSlider = nullptr;
    QLabel* _trackPlayStateLbl = nullptr;
    QString _playStateDescriptor;

    void _setPlayButtonState(bool isPlaying);
    void _tooglePlayButtonState();
    void _updateTrackTimeStateDescriptor(int stateInSeconds);

    bool _sliderDown = false;
};
