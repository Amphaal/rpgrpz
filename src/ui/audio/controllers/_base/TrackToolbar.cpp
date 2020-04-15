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
// different license and copyright still refer to this GNU General Public License.

#include "TrackToolbar.h"

TrackToolbar::TrackToolbar(QWidget* parent) : QWidget(parent), 
    _playBtn(new QToolButton), 
    _rewindBtn(new QToolButton),
    _forwardBtn(new QToolButton), 
    _trackStateSlider(new QSlider(Qt::Orientation::Horizontal)), 
    _trackPlayStateLbl(new QLabel) {
    
    this->_playBtn->setCheckable(true);

    this->endTrack();

    //play / pause
    QObject::connect(
        this->_playBtn, &QAbstractButton::clicked,
        this, &TrackToolbar::_tooglePlayButtonState
    );
    
    //rewind
    this->_rewindBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/audio/rewind.png")));
    QObject::connect(
        this->_rewindBtn, &QAbstractButton::clicked,
        [&]() {
            emit actionRequired(TrackToolbar::Action::Rewind); 
        }
    );

    //forward
    this->_forwardBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/audio/forward.png")));
    QObject::connect(
        this->_forwardBtn, &QAbstractButton::clicked,
        [&]() {
            emit actionRequired(TrackToolbar::Action::Forward);
        }
    );

    //on slider release
    QObject::connect(
        this->_trackStateSlider, &QSlider::sliderReleased,
        [&]() {
            this->_sliderDown = false;
            auto pos = this->_trackStateSlider->value();
            emit seeking(pos);
        }
    );
    QObject::connect(
        this->_trackStateSlider, &QSlider::sliderPressed,
        [&]() {
            this->_sliderDown = true;
        }
    );

    QObject::connect(
        this->_trackStateSlider, &QSlider::valueChanged,
        [&](int pos) {
           
            this->_updateTrackTimeStateDescriptor(pos);
            
            if(!this->_sliderDown) {
                emit seeking(pos);
            }

        }
    );

    //layout
    this->setLayout(new QHBoxLayout);
    this->layout()->addWidget(this->_rewindBtn);
    this->layout()->addWidget(this->_playBtn);
    this->layout()->addWidget(this->_forwardBtn);
    this->layout()->addWidget(this->_trackStateSlider);
    this->layout()->addWidget(this->_trackPlayStateLbl);

}

void TrackToolbar::_updateTrackTimeStateDescriptor(int stateInSeconds) {
    
    //formated
    auto current = stateInSeconds < 0 ? TrackToolbar::_defaultNoTime : StringHelper::fromSecondsToTime(stateInSeconds);
    
    //update lbl
    this->_trackPlayStateLbl->setText(
        this->_playStateDescriptor.arg(current)
    );

}

void TrackToolbar::updatePlayerPosition(int posInSeconds) {
    
    //prevent updating while slider is manipulated
    if(this->_sliderDown) return;

    //conditionnal state widgets
    auto enabledWidgets = posInSeconds > -1;
    this->_trackStateSlider->setEnabled(enabledWidgets);
    
    //update slider value
    {
        QSignalBlocker b(this->_trackStateSlider);
        this->_trackStateSlider->setValue(posInSeconds);
    }

    this->_playBtn->setEnabled(enabledWidgets);

    this->_rewindBtn->setEnabled(enabledWidgets);
    this->_forwardBtn->setEnabled(enabledWidgets);

    //update txt val
    this->_updateTrackTimeStateDescriptor(posInSeconds);

}

void TrackToolbar::endTrack() {
    //update descriptor and UI state
    this->_playStateDescriptor = _trackPlayStateTemplator.arg("%1", _defaultNoTime);
    this->updatePlayerPosition(-1);
    this->_setPlayButtonState(false);
}


void TrackToolbar::newTrack(int lengthInSeconds) {
    
    //update without triggering events
    {
        QSignalBlocker b(this->_trackStateSlider);
        this->_trackStateSlider->setValue(0);
        this->_trackStateSlider->setMaximum(lengthInSeconds);
    }

    //update descriptor and UI state
    this->_playStateDescriptor = _trackPlayStateTemplator.arg("%1", StringHelper::fromSecondsToTime(lengthInSeconds));
    this->updatePlayerPosition(0);
    this->_setPlayButtonState(true);

}

void TrackToolbar::_setPlayButtonState(bool isPlaying) {
    this->_playBtn->setChecked(isPlaying);
    this->_playBtn->setIcon(isPlaying ? this->_pauseIcon : this->_playIcon);
}

void TrackToolbar::_tooglePlayButtonState() {
    
    auto btnstate = this->_playBtn->isChecked();
    this->_setPlayButtonState(btnstate);
    
    if(btnstate) {
        emit actionRequired(TrackToolbar::Action::Play);
    } else {
        emit actionRequired(TrackToolbar::Action::Pause);
    }

}