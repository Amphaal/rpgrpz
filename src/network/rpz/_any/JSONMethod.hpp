#pragma once

#include <QString>

enum JSONMethod { 
    MessageFromPlayer,
    ServerResponse, 
    PlayerHasUsername, 
    LoggedPlayersChanged, 
    ChatLogHistory,
    MapChanged,
    AskForHostMapHistory,
    AckIdentity,
    AskForAsset,
    RequestedAsset 
};

static const QStringList JSONMethodAsArray = { 
    "MessageFromPlayer",
    "ServerResponse", 
    "PlayerHasUsername", 
    "LoggedPlayersChanged", 
    "ChatLogHistory",
    "MapChanged",
    "AskForHostMapHistory",
    "AckIdentity",
    "AskForAsset",
    "RequestedAsset"
};