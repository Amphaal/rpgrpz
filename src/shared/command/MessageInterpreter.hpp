#pragma once

#include <QHash>
#include <QString>

class MessageInterpreter {
    public:
        enum MessageInterpretation { Unknown, Say, Whisper };
        static inline QHash<QString, MessageInterpretation> _commandByInterpretation = {
            { "/w", Whisper }
        };

        static MessageInterpretation findInterpretation(QString &text) {
            //initial trim
            text = text.trimmed();
            
            //if not a command
            if(!text.startsWith("/")) return MessageInterpretation::Say;

            //extract command
            auto command = text.split(" ", QString::SplitBehavior::SkipEmptyParts)[0];

            //returns command   
            return _commandByInterpretation[command];
        }
};