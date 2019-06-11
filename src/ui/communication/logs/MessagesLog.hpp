#pragma once

#include "base/LogContainer.h"

#include "src/shared/models/RPZMessage.hpp"
#include "src/shared/models/RPZResponse.hpp"

class MessagesLog : public LogContainer {

    public:
        MessagesLog(QWidget *parent = nullptr) : LogContainer(parent) {};

        void handleResponse(RPZResponse &response) {

            auto respToId = response.answerer();

            //if respond to a message, "ungrey" the responded
            if(respToId) {
                
                auto existingLine = LogContainer::_getLine(respToId);
                auto existingPalette = existingLine->palette();
                
                auto txtColor = existingPalette.color(QPalette::WindowText);
                txtColor.setAlpha(255);
                existingPalette.setColor(QPalette::WindowText, txtColor);

                existingLine->setPalette(existingPalette);

            }

            //if response code is ask, stop here
            auto respCode = response.responseCode();
            if(respCode == RPZResponse::Ack) return;

            //get new line
            auto newLine = LogContainer::_addLine(response, respToId);
            
            //add text
            auto txt = new LogText(response.toString());
            newLine->horizontalLayout()->addWidget(txt, 10);

            
            //set palette
            newLine->setPalette(response.palette());

        }

        void handleMessage(RPZMessage &msg, bool isLocal = false) {
            
            //should not exist
            auto targetLine = LogContainer::_getLine(msg);
            if(targetLine) return;

            //create new line
            targetLine = LogContainer::_addLine(msg);
                
            //add text
            auto txt = new LogText(msg.toString());
            targetLine->horizontalLayout()->addWidget(txt, 10);

            //define palette to apply
            auto msgPalette = msg.palette();
            
            //if is local, expect a server response and add opacity to signal it to the user
            if(isLocal) {
                auto txtColor = msgPalette.color(QPalette::WindowText);
                txtColor.setAlpha(128);
                msgPalette.setColor(QPalette::WindowText, txtColor);
            }

            //apply it
            targetLine->setPalette(msgPalette);
        }

};