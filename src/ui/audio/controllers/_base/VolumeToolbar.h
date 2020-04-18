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

#include <QWidget>
#include <QSlider>
#include <QToolButton>
#include <QIcon>
#include <QLabel>

#include <QHBoxLayout>

#include "src/helpers/_appContext.h"

class VolumeToolbar : public QWidget {
    
    Q_OBJECT

    signals:
        void askForVolumeChange(int newVolume);

 public:
        VolumeToolbar(QWidget* parent = nullptr);
    
 private:
        QSlider* _audio = nullptr;
        QToolButton* _mute = nullptr;
        QLabel* _volumeStr = nullptr;

        QIcon _volumeIcon = QIcon(QStringLiteral(u":/icons/app/audio/volume.png"));
        QIcon _mutedIcon = QIcon(QStringLiteral(u":/icons/app/audio/mute.png"));

        void _onAudioChange(int newSliderVal);
        void _setAudioValLbl(int sliderVal);
        void _onMuteButtonClick();
};