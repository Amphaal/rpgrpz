#pragma once

#include <QString>

enum JSONMethod { 
    MessageFromPlayer, 
    PlayerHasUsername, 
    LoggedPlayersChanged, 
    ChatLogHistory,
    HostMapHistory,
    HostMapChanged,
    AskForHostMapHistory 
};

static const QString JSONMethodAsArray[] = { 
    "MessageFromPlayer", 
    "PlayerHasUsername", 
    "LoggedPlayersChanged", 
    "ChatLogHistory",
    "HostMapHistory",
    "HostMapChanged",
    "AskForHostMapHistory"
};