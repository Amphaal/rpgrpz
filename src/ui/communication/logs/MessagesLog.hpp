#pragma once

#include "base/LogContainer.h"

#include "src/shared/models/entities/RPZMessage.hpp"

class MessagesLog : public LogContainer {

    public:
        enum MessageType { Default, ServerLog, ClientMessage, ClientMessageWaiting };

        MessagesLog(QWidget *parent = nullptr) : LogContainer(parent) {};

        void handleMessage(RPZMessage &msg) {

            //get line
            MessageType msgType;
            auto respondedToId = msg.respondTo();
            auto targetLine = LogContainer::_getLine(msg);

            //create if line not found
            if(!targetLine) {
                
                //create line
                targetLine = LogContainer::_addLine(msg, respondedToId);
                
                //add text
                auto txt = new LogText(msg.toString());
                targetLine->horizontalLayout()->addWidget(txt, 10);

                //define msg type
                msgType = msg.owner().isEmpty() ? ServerLog : ClientMessageWaiting;
            } 
            
            //if found, ack message received
            else {
                msgType = ClientMessage;
            }

            //update palette 
            this->_updateLogItemPalette(targetLine, msgType);

            //if respondedToId, update its palette too
            if(!respondedToId.isNull()) {
                this->_updateLogItemPalette(LogContainer::_getLine(respondedToId), ClientMessage);
            }
        }

    private:
        void _updateLogItemPalette(LogItem * item, const MessageType &logType) {
    
            QPalette colors = QPalette();

            switch(logType) {
                case MessageType::ServerLog:
                    colors.setColor(QPalette::Window, "#71ed55");
                    colors.setColor(QPalette::WindowText, Qt::black);
                    break;
                case MessageType::ClientMessageWaiting:
                    colors.setColor(QPalette::WindowText, "#999999");
                    break;
            }

            item->setPalette(colors);
        };

};