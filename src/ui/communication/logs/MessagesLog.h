#pragma once

#include "base/LogContainer.h"

#include "src/shared/models/RPZMessage.hpp"
#include "src/shared/models/RPZResponse.hpp"

class MessagesLog : public LogContainer {
    public:
        MessagesLog(QWidget *parent = nullptr);

        void handleResponse(RPZResponse &response);
        void handleMessage(RPZMessage &msg, bool isLocal = false);
};