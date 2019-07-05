#pragma once

#include <QString>

enum JSONMethod { 
    MessageFromPlayer,
    ServerStatus,
    ServerResponse, 
    Handshake, 
    LoggedPlayersChanged, 
    ChatLogHistory,
    MapChanged,
    AskForHostMapHistory,
    AckIdentity,
    AskForAsset,
    RequestedAsset,
    AudioStreamChanged 
};

static const QStringList JSONMethodAsArray = { 
    "MessageFromPlayer",
    "ServerStatus",
    "ServerResponse", 
    "Handshake", 
    "LoggedPlayersChanged", 
    "ChatLogHistory",
    "MapChanged",
    "AskForHostMapHistory",
    "AckIdentity",
    "AskForAsset",
    "RequestedAsset",
    "AudioStreamChanged"
};