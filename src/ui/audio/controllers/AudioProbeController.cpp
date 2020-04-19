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

#include "AudioProbeController.h"

AudioProbeController::AudioProbeController(QWidget * parent) : QGroupBox(tr("Playing now"), parent), 
    toolbar(new VolumeToolbar),
    _descr(new QLabel),
    _playIconLbl(new QLabel),
    _posInTrackLbl(new QLabel),
    _playIcon(new QMovie(":/icons/app/audio/musicPlaying.gif")) {

    //bind animation to label
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    this->setLayoutDirection(Qt::LeftToRight);
    this->setAlignment(Qt::AlignHCenter);
    this->_playIconLbl->setMovie(this->_playIcon);
    
    //layout
        auto h = new QHBoxLayout;
        h->setMargin(0);
        h->addWidget(this->_posInTrackLbl);
        h->addStretch(1);
        h->addWidget(this->toolbar, 3);

        auto v = new QVBoxLayout;
        v->setMargin(0);
        v->addWidget(this->_descr);
        v->addLayout(h);

        auto topMostLayout = new QHBoxLayout;
        this->setLayout(topMostLayout);
        topMostLayout->addWidget(this->_playIconLbl);
        topMostLayout->addLayout(v, 1);
    //END layout

    this->updatePlayedMusic(NULL);
    this->changeTrackPosition(-1);

}

void AudioProbeController::changeTrackPosition(int posInSecs) {
    auto text = posInSecs < 0 ? _defaultNoTime : StringHelper::secondsToTrackDuration(posInSecs);
    this->_posInTrackLbl->setText(text);
}

void AudioProbeController::updatePlayedMusic(const QString &musicName) {
    
    auto musicIsPlaying = !musicName.isNull();
    
    if(!musicIsPlaying) {
        this->_descr->setText(tr("Nothing playing"));
        this->_playIcon->stop();
    }
    else {
        this->_descr->setText(musicName);
        this->_playIcon->start();
    }

    this->changeTrackPosition(-1);
    this->_posInTrackLbl->setVisible(musicIsPlaying);
    this->_playIconLbl->setVisible(musicIsPlaying);
    this->toolbar->setVisible(musicIsPlaying);
    
    this->_descr->setEnabled(musicIsPlaying);
    
}

void AudioProbeController::changeTrackState(bool isPlaying) {
    if(isPlaying) this->_playIcon->start();
    else this->_playIcon->stop();
}