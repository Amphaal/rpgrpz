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

#include "ViewMapHint.h"
#include "src/ui/_others/ConnectivityObserver.h"

class MapHint : public ViewMapHint, public ConnectivityObserver {

    Q_OBJECT

 public:
        MapHint();

        //load/unload
        bool isRemote() const;
        bool isMapDirty() const;
        const QString mapFilePath() const;

        void mayWantToSavePendingState(QWidget* parent); //must block UI
        
        bool ackRemoteness(const RPZUser &connectedUser, const QString &remoteAddress);
        bool ackRemoteness(const QString &tblMapFilePath);

    public slots:
        bool loadDefaultRPZMap(); //to invoke
        bool loadRPZMap(const QString &filePath); //to invoke
        bool saveRPZMap(); //to invoke, unless from mayWantToSavePendingState()
        bool saveRPZMapAs(const QString &newFilePath); //to invoke
        bool createNewRPZMapAs(const QString &newFilePath); //to invoke

    signals:
        void mapStateChanged(const QString &mapDescriptor, bool isMapDirty);
        void remoteChanged(bool isRemote);

 private: 
        bool _ackRemoteness();

        QString _mapDescriptor;

        bool _isRemote = false;
        bool _isMapDirty = false;

        void _setMapDirtiness(bool dirty = true);
        void _shouldMakeMapDirty(const AlterationPayload &payload);
    
        void _handleAlterationRequest(const AlterationPayload &payload) final;

        AlterationInteractor* _sysActor = nullptr;
        
        //network
            void connectingToServer() override;
            void connectionClosed(bool hasInitialMapLoaded) override;

            void _onGameSessionReceived(const RPZGameSession &gameSession);
            void _mightUpdateTokens();
            void _sendMapHistory();

};