#pragma once

#include "base/LogScrollView.h"

class MessagesLog : public LogScrollView {

    public:
        enum MessageType { Default, ServerLog, ClientMessage };

        void writeAtEnd(const QString &newMessage, const MessageType &logType = MessageType::Default) {
            
            auto line = LogScrollView::writeAtEnd(newMessage);
            if(!line) return;

            line->setPalette(this->getPalette(logType));
        }

    private:
        QPalette getPalette(const MessageType &logType) {
    
            QPalette colors = QPalette();

            switch(logType) {
                case MessageType::ServerLog:
                    colors.setColor(QPalette::Window, "#71ed55");
                    colors.setColor(QPalette::WindowText, Qt::black);
                    break;
            }

            return colors;
        };

};