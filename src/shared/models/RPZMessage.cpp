#include "RPZMessage.h"

RPZMessage::RPZMessage() {};
RPZMessage::RPZMessage(const QVariantHash &hash) : Stampable(hash) {
    this->_interpretTextAsCommand();
}

RPZMessage::RPZMessage(const QString &message, const MessageInterpreter::Command &forceCommand) : Stampable() { 
    if(forceCommand) this->_forceCommand(forceCommand);
    this->_setText(message);
};

QString RPZMessage::text() {
    return this->value("txt").toString();
}

MessageInterpreter::Command RPZMessage::commandType() {
    return this->_command;
}

QString RPZMessage::toString() {

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

QPalette RPZMessage::palette() {
    
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

void RPZMessage::_setText(const QString &text) {
    this->insert("txt", text);
    this->_interpretTextAsCommand();
}

void RPZMessage::_forceCommand(const MessageInterpreter::Command &forced) {
    this->insert("cmd", (int)forced);
}

void RPZMessage::_interpretTextAsCommand() {
    auto forcedCommand = (MessageInterpreter::Command)this->value("cmd").toInt();
    this->_command = forcedCommand ? forcedCommand : MessageInterpreter::interpretText(this->text());
}