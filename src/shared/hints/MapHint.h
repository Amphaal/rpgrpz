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

#include "ViewMapHint.h"

class MapHint : public ViewMapHint {

    Q_OBJECT

    public:
        MapHint();

        //load/unload
        bool isRemote() const;
        bool isMapDirty() const;
        const QString mapFilePath() const;

        static void mayWantToSavePendingState(QWidget* parent, MapHint* hint); //must block UI
        
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

    private: 
        bool _ackRemoteness();

        QString _mapDescriptor;

        bool _isRemote = false;
        bool _isMapDirty = false;

        void _setMapDirtiness(bool dirty = true);
        void _shouldMakeMapDirty(const AlterationPayload &payload);
    
        void _handleAlterationRequest(const AlterationPayload &payload) final;

        AlterationActor* _sysActor = nullptr;
};