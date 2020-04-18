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

#include <QObject>

class ProgressTracker : public QObject {
    
    Q_OBJECT
    
 public:
        enum class Kind { Map, Asset };
        Q_ENUM(Kind)

        static ProgressTracker* get() {
            if(!_inst) _inst = new ProgressTracker;
            return _inst; 
        }

    public slots:
        void heavyAlterationStarted() {
            emit heavyAlterationProcessing();
        }

        void heavyAlterationEnded() {
            emit heavyAlterationProcessed();
        }

        void serverIsActive() {
            emit serverActive();
        }

        void serverIsInactive() {
            emit serverInactive();
        }

        void clientIsSending() {
            emit clientSending();
        }

        void clientStoppedSending() {
            emit clientSent();
        }

        void clientIsReceiving() {
            emit clientReceiving();
        }

        void clientStoppedReceiving() {
            emit clientReceived();
        }

        void downloadIsStarting(const ProgressTracker::Kind &kind, qint64 size) {
            emit downloadStarted(kind, size);
        }

        void downloadIsProgressing(const ProgressTracker::Kind &kind, qint64 progress) {
            emit downloadProgress(kind, progress);
        }

        void downloadHasEnded(const ProgressTracker::Kind &kind) {
            emit downloadEnded(kind);
        }

    signals:
        void heavyAlterationProcessing();
        void heavyAlterationProcessed();
        
        void serverActive();
        void serverInactive();
        
        void clientSending();
        void clientSent();

        void clientReceiving();
        void clientReceived();

        void downloadStarted(const ProgressTracker::Kind &kind, qint64 size);
        void downloadProgress(const ProgressTracker::Kind &kind, qint64 progress);
        void downloadEnded(const ProgressTracker::Kind &kind);
    
 private:
        static inline ProgressTracker* _inst = nullptr;

};

        