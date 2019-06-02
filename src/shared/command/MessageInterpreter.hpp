#pragma once

#include <QHash>
#include <QString>

class MessageInterpreter {
    public:
        enum Command { Unknown, Say, Whisper, Help };
        
        static inline QHash<QString, Command> _textByCommand = {
            { "/w", Whisper },
            { "/h", Help }
        };

        static Command interpretText(QString &text) {
            //initial trim
            text = text.trimmed();
            
            //if not a command
            if(!text.startsWith("/")) return Command::Say;

            //extract command
            auto command = text.split(" ", QString::SplitBehavior::SkipEmptyParts)[0];

            //returns command   
            return _textByCommand[command];
        }

        static QString help()  {
            return "Pas d'aide disponible";
        }
};