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

#include <QTcpSocket>
#include <QString>
#include <QVariant>

class RPZJSON {
    Q_GADGET

 public:
    enum class Method {
        M_Unknown = 0,
        Handshake,
        Message,
        ServerStatus,
        ServerResponse,
        MapChanged,
        MapChangedHeavily,
        AskForAssets,
        RequestedAsset,
        AudioStreamUrlChanged,
        AudioStreamPlayingStateChanged,
        AudioStreamPositionChanged,
        AvailableAssetsToUpload,
        UserIn,
        UserOut,
        CharacterChanged,
        UserDataChanged,
        QuickDrawHappened,
        GameSessionSync,
        SharedDocumentAvailable,
        SharedDocumentRequested,
        PingHappened
    };
    Q_ENUM(Method)

    static bool mayBeHeavyPayload(const RPZJSON::Method &method) {
        return _heavyPayloadMethods.contains(method);
    }

 private:
    static inline QList<RPZJSON::Method> _heavyPayloadMethods {
        RPZJSON::Method::MapChangedHeavily,
        RPZJSON::Method::RequestedAsset,
        RPZJSON::Method::SharedDocumentRequested,
        RPZJSON::Method::GameSessionSync
    };
};
