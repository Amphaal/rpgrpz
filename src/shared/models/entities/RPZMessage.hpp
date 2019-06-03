#pragma once 

#include <QString>
#include <QUuid>
#include <QDateTime>

#include "RPZUser.hpp"
#include "base/Stampable.hpp"

#include "src/shared/command/MessageInterpreter.hpp"

class RPZMessage : public Stampable {
    
    public:
        RPZMessage() {};
        RPZMessage(const QVariantHash &hash) : Stampable(hash) {
            this->_interpretTextAsCommand();
        }

        RPZMessage(const QString &message) : Stampable() { 
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

            switch(this->_command) {
                case MessageInterpreter::Say:
                    return base + " a dit : " + "“" + this->text() +  "”";

                default:
                    return this->text();
            }

        };

    private:
        MessageInterpreter::Command _command = MessageInterpreter::Command::Unknown;
        
        void _setText(const QString &text) {
            (*this)["txt"] = text;
            this->_interpretTextAsCommand();
        }

        void _interpretTextAsCommand() {
            this->_command = MessageInterpreter::interpretText(this->text());
        }
};