#pragma once

#include <QString>

enum JSONMethod { 
    MessageFromPlayer, 
    PlayerHasUsername, 
    LoggedPlayersChanged, 
    ChatLogHistory,
    HostMapHistory,
    MapChanged,
    AskForHostMapHistory,
    AckIdentity,
    AskForAsset,
    RequestedAsset 
};

static const QStringList JSONMethodAsArray = { 
    "MessageFromPlayer", 
    "PlayerHasUsername", 
    "LoggedPlayersChanged", 
    "ChatLogHistory",
    "HostMapHistory",
    "MapChanged",
    "AskForHostMapHistory",
    "AckIdentity",
    "AskForAsset",
    "RequestedAsset"
};