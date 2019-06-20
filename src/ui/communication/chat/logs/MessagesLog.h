#pragma once

#include "base/LogContainer.h"

#include "src/shared/models/RPZMessage.h"
#include "src/shared/models/RPZResponse.h"

class MessagesLog : public LogContainer {
    public:
        MessagesLog(QWidget *parent = nullptr);

        void handleResponse(RPZResponse &response);
        void handleMessage(RPZMessage &msg, bool isLocal = false);
};