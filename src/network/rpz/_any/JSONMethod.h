#pragma once

#include <QString>

enum JSONMethod { 
    MessageFromPlayer, 
    PlayerHasUsername, 
    LoggedPlayersChanged, 
    ChatLogHistory 
};

static QString JSONMethodAsArray[] = { "MessageFromPlayer", "PlayerHasUsername", "LoggedPlayersChanged", "ChatLogHistory" };