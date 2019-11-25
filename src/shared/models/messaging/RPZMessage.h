#pragma once 

#include <QString>
#include <QDateTime>

#include "src/shared/models/_base/RPZMap.hpp"
#include "src/shared/models/RPZUser.h"
#include "src/shared/models/_base/Stampable.hpp"
#include "src/shared/commands/MessageInterpreter.h"

class RPZMessage : public Stampable {
    
    public:
        RPZMessage();
        explicit RPZMessage(const QVariantHash &hash);
        RPZMessage(const QString &message, const MessageInterpreter::Command &forceCommand = MessageInterpreter::Command::C_Unknown);

        QString text() const;
        MessageInterpreter::Command commandType() const;
        QString toString() const override;
        QPalette palette() const override;

    private:
        MessageInterpreter::Command _command = MessageInterpreter::Command::C_Unknown;
        
        void _setText(const QString &text);
        void _forceCommand(const MessageInterpreter::Command &forced);
        void _interpretTextAsCommand();
};