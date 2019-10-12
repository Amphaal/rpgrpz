#include "RPZMessage.h"

RPZMessage::RPZMessage() {};
RPZMessage::RPZMessage(const QVariantHash &hash) : Stampable(hash) {
    this->_interpretTextAsCommand();
}

RPZMessage::RPZMessage(const QString &message, const MessageInterpreter::Command &forceCommand) : Stampable() { 
    if(forceCommand) this->_forceCommand(forceCommand);
    this->_setText(message);
};

QString RPZMessage::text() const {
    return this->value("txt").toString();
}

MessageInterpreter::Command RPZMessage::commandType() const {
    return this->_command;
}

QString RPZMessage::toString() const {

    auto base = Stampable::toString();
    auto ownerExist = !this->owner().isEmpty();
    auto text = this->text();

    switch(this->_command) {
        
        case MessageInterpreter::Say: {
            QString textPrefix = ownerExist ? QObject::tr(" said : ") : QObject::tr("you said : ");
            return base + textPrefix + QChar(0x201C) + text + QChar(0x201D);
        }

        case MessageInterpreter::Whisper: {
            QString textPrefix = QObject::tr(" whispers to you : ");
            
            if(!ownerExist) {
                auto recipientList = MessageInterpreter::findRecipentsFromText(text).join(", ");
                textPrefix = QObject::tr("your whisper to ") + recipientList + " : ";
                text = MessageInterpreter::sanitizeText(text);
            }

            return base + textPrefix + QChar(0x201C) + text + QChar(0x201D);
        }

        default:
            return text;
            
    }

};

QPalette RPZMessage::palette() const {
    
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
        
        case MessageInterpreter::C_DiceThrow:
            palette.setColor(QPalette::Window, "#87CEEB");
            palette.setColor(QPalette::WindowText, "#000080");
            break;

        default:
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
