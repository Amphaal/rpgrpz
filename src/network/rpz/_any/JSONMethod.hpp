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
    AckIdentity 
};

static const QStringList JSONMethodAsArray = { 
    "MessageFromPlayer", 
    "PlayerHasUsername", 
    "LoggedPlayersChanged", 
    "ChatLogHistory",
    "HostMapHistory",
    "MapChanged",
    "AskForHostMapHistory",
    "AckIdentity"
};