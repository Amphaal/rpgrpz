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
    AskForAssets,
    RequestedAsset,
    AudioStreamUrlChanged,
    AudioStreamPlayingStateChanged,
    AudioStreamPositionChanged,
    MapChangedHeavily,
    AvailableAssetsToUpload
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
    "AskForAssets",
    "RequestedAsset",
    "AudioStreamUrlChanged",
    "AudioStreamPlayingStateChanged",
    "AudioStreamPositionChanged",
    "MapChangedHeavily",
    "AvailableAssetsToUpload"
};
