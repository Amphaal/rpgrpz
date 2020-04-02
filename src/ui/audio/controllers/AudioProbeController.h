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
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>

#include "src/ui/audio/controllers/_base/VolumeToolbar.h"
#include "src/helpers/StringHelper.hpp"

class AudioProbeController : public QGroupBox {

    Q_OBJECT

    public:
        AudioProbeController(QWidget * parent = nullptr);
        
        void updatePlayedMusic(const QString &musicName);
        void changeTrackState(bool isPlaying);
        void changeTrackPosition(int posInSecs);
        
        VolumeToolbar* toolbar = nullptr;
        
    private:
        static inline const QString _defaultNoTime = "--"; 

        QLabel* _descr = nullptr;
        QLabel* _playIconLbl = nullptr;
        QLabel* _posInTrackLbl = nullptr;
        QMovie* _playIcon = nullptr;
};