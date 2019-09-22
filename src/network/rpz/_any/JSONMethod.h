#pragma once

#include <QString>

enum JSONMethod { 
    Message,
    ServerStatus,
    ServerResponse, 
    Handshake, 
    LoggedPlayersChanged, 
    ChatLogHistory,
    MapChanged,
    AskForHostMapHistory,
    AckIdentity,
    AskForAssets,
    RequestedAsset,
    AudioStreamUrlChanged,
    AudioStreamPlayingStateChanged,
    AudioStreamPositionChanged,
    MapChangedHeavily,
    AvailableAssetsToUpload
};

static const QStringList JSONMethodAsArray = { 
    "Message",
    "ServerStatus",
    "ServerResponse", 
    "Handshake", 
    "LoggedPlayersChanged", 
    "ChatLogHistory",
    "MapChanged",
    "AskForHostMapHistory",
    "AckIdentity",
    "AskForAssets",
    "RequestedAsset",
    "AudioStreamUrlChanged",
    "AudioStreamPlayingStateChanged",
    "AudioStreamPositionChanged",
    "MapChangedHeavily",
    "AvailableAssetsToUpload"
};
