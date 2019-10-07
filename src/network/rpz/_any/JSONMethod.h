#pragma once

#include <QString>

enum JSONMethod { 
    Message,
    ServerStatus,
    ServerResponse, 
    Handshake, 
    AllConnectedUsers, 
    ChatLogHistory,
    MapChanged,
    AckIdentity,
    AskForAssets,
    RequestedAsset,
    AudioStreamUrlChanged,
    AudioStreamPlayingStateChanged,
    AudioStreamPositionChanged,
    MapChangedHeavily,
    AvailableAssetsToUpload,
    UserIn,
    UserOut
};

static const QStringList JSONMethodAsArray = { 
    "Message",
    "ServerStatus",
    "ServerResponse", 
    "Handshake", 
    "AllConnectedUsers", 
    "ChatLogHistory",
    "MapChanged",
    "AckIdentity",
    "AskForAssets",
    "RequestedAsset",
    "AudioStreamUrlChanged",
    "AudioStreamPlayingStateChanged",
    "AudioStreamPositionChanged",
    "MapChangedHeavily",
    "AvailableAssetsToUpload",
    "UserIn",
    "UserOut"
};
