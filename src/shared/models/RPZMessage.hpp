#pragma once 

#include <QString>
#include <QDateTime>

#include "RPZUser.hpp"
#include "base/Stampable.hpp"

#include "src/shared/commands/MessageInterpreter.h"

class RPZMessage : public Stampable {
    
    public:
        RPZMessage() {};
        RPZMessage(const QVariantHash &hash) : Stampable(hash) {
            this->_interpretTextAsCommand();
        }

        RPZMessage(const QString &message, const MessageInterpreter::Command &forceCommand = MessageInterpreter::Unknown) : Stampable() { 
            if(forceCommand) this->_forceCommand(forceCommand);
            this->_setText(message);
        };

        QString text() {
            return this->value("txt").toString();
        }

        MessageInterpreter::Command commandType() {
            return this->_command;
        }

        QString toString() override {

            auto base = Stampable::toString();
            auto ownerExist = !this->owner().isEmpty();
            auto text = this->text();

            switch(this->_command) {
                case MessageInterpreter::Say: {
                    QString textPrefix = ownerExist ? " a dit : " : "vous dites : ";
                    return base + textPrefix + "“" + text +  "”";
                }

                case MessageInterpreter::Whisper: {
                    QString textPrefix = " vous chuchotte : ";
                    
                    if(!ownerExist) {
                        auto recipientList = MessageInterpreter::findRecipentsFromText(text).join(", ");
                        textPrefix = QString("vous chuchottez à ") + recipientList + " : ";
                        text = MessageInterpreter::sanitizeText(text);
                    }

                    return base + textPrefix + "“" + text +  "”";
                }

                default:
                    return text;
            }

        };

        QPalette palette() override {
            
            //default palette
            auto palette = Stampable::palette();

            //switch by resp code...
            switch(this->_command) {
                
                case MessageInterpreter::Whisper:
                    palette.setColor(QPalette::Window, "#f2e8f9");
                    palette.setColor(QPalette::WindowText, "#a12ded");
                    break;
                
                case MessageInterpreter::Say:
                    palette.setColor(QPalette::Window, "#FFFFFF");
                    palette.setColor(QPalette::WindowText, "#000000");
                    break;
            }

            return palette;
        
        }

    private:
        MessageInterpreter::Command _command = MessageInterpreter::Command::Unknown;
        
        void _setText(const QString &text) {
            (*this)["txt"] = text;
            this->_interpretTextAsCommand();
        }

        void _forceCommand(const MessageInterpreter::Command &forced) {
            (*this)["cmd"] = (int)forced;
        }

        void _interpretTextAsCommand() {
            auto forcedCommand = (MessageInterpreter::Command)this->value("cmd").toInt();
            this->_command = forcedCommand ? forcedCommand : MessageInterpreter::interpretText(this->text());
        }
};