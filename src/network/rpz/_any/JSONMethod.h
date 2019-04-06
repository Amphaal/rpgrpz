#pragma once

#include <QString>

enum JSONMethod { 
    MessageFromPlayer, 
    PlayerHasUsername, 
    LoggedPlayersChanged, 
    ChatLogHistory,
    HostMapHistory,
    HostMapChanged 
};

static QString JSONMethodAsArray[] = { 
    "MessageFromPlayer", 
    "PlayerHasUsername", 
    "LoggedPlayersChanged", 
    "ChatLogHistory",
    "HostMapHistory",
    "HostMapChanged" 
};