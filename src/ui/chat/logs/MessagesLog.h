#pragma once

#include "base/LogContainer.h"

#include "src/shared/models/RPZMessage.h"
#include "src/shared/models/RPZResponse.h"

#include <QBoxLayout>

class MessagesLog : public LogContainer {
    public:
        MessagesLog(QWidget *parent = nullptr);

        void handleResponse(const RPZResponse &response);
        
        void handleLocalMessage(const RPZMessage &msg);
        void handleNonLocalMessage(const RPZMessage &msg);
    
    private:
        void _handleMessage(const RPZMessage &msg, bool isLocal = false);

        void changeEvent(QEvent *event) override;
};