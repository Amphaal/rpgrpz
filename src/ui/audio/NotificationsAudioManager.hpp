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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.


#pragma once

#include <QSoundEffect>

class NotificationsAudioManager {
    public:
        static NotificationsAudioManager* get() {
            if(!_instance) _instance = new NotificationsAudioManager();
            return _instance;
        }

        void playDiceThrow() {
            auto effect = new QSoundEffect;

            auto randomIndex = QRandomGenerator::global()->bounded(0, _diceEffects.count() - 1);
            auto source = _diceEffects.value(randomIndex);

            effect->setSource(QUrl::fromLocalFile(source));
            effect->setVolume(.5);
            effect->play();

            QObject::connect(
                effect, &QSoundEffect::playingChanged, 
                effect, &QObject::deleteLater
            );
            
        }

        void playUserIn()

        void playUserOut()

        void playNewMessage()

        void playWhisper()

    private:
        NotificationsAudioManager() {}

        static inline const QList<QString> _diceEffects {
            ":/audio/dice1.wav",
            ":/audio/dice2.wav",
            ":/audio/dice3.wav",
            ":/audio/dice4.wav"
        };

        static inline NotificationsAudioManager* _instance = nullptr;
};