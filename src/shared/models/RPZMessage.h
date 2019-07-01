#pragma once 

#include <QString>
#include <QDateTime>

#include "RPZUser.h"
#include "base/Stampable.hpp"

#include "src/shared/commands/MessageInterpreter.h"

class RPZMessage : public Stampable {
    
    public:
        RPZMessage();
        RPZMessage(const QVariantHash &hash);
        RPZMessage(const QString &message, const MessageInterpreter::Command &forceCommand = MessageInterpreter::C_Unknown);

        QString text();
        MessageInterpreter::Command commandType();
        QString toString() override;
        QPalette palette() override;

    private:
        MessageInterpreter::Command _command = MessageInterpreter::Command::C_Unknown;
        
        void _setText(const QString &text);
        void _forceCommand(const MessageInterpreter::Command &forced);
        void _interpretTextAsCommand();
};