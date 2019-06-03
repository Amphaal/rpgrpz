#pragma once

#include "base/LogContainer.h"

#include "src/shared/models/entities/RPZMessage.hpp"
#include "src/shared/models/entities/RPZResponse.hpp"

class MessagesLog : public LogContainer {

    public:
        enum MessageStyle { Default, ServerLog, ClientMessageWaiting };

        MessagesLog(QWidget *parent = nullptr) : LogContainer(parent) {};

        void handleResponse(RPZResponse &response) {

            auto respToId = response.answerer();
            
            //update style of response Item
            if(!respToId.isNull()) {
                auto existingLine = LogContainer::_getLine(respToId);
                this->_updateLogItemPalette(existingLine);
            }

            //depending on response code
            switch(response.responseCode()) {

                case RPZResponse::Ack: {
                    //nothing to do
                }
                break;
                
                default: {

                    //get new line
                    auto newLine = LogContainer::_addLine(response, respToId);
                    
                    //add text
                    auto txt = new LogText(response.toString());
                    newLine->horizontalLayout()->addWidget(txt, 10);

                    //define style as server log
                    this->_updateLogItemPalette(newLine, ServerLog);

                }
                break;
            }

        }

        void handleMessage(RPZMessage &msg) {
            
            //should not exist
            auto targetLine = LogContainer::_getLine(msg);
            if(targetLine) return;

            //create new line
            targetLine = LogContainer::_addLine(msg);
                
            //add text
            auto txt = new LogText(msg.toString());
            targetLine->horizontalLayout()->addWidget(txt, 10);

            //set waiting palette
            this->_updateLogItemPalette(targetLine, ClientMessageWaiting);

        }

    private:
        void _updateLogItemPalette(LogItem * item, const MessageStyle &logType = Default) {
    
            QPalette colors;

            switch(logType) {
                case MessageStyle::ServerLog:
                    colors.setColor(QPalette::Window, "#71ed55");
                    colors.setColor(QPalette::WindowText, Qt::black);
                    break;
                case MessageStyle::ClientMessageWaiting:
                    colors.setColor(QPalette::WindowText, "#999999");
                    break;
            }

            item->setPalette(colors);
        };

};