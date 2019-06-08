#pragma once

#include <QHash>
#include <QString>

#include <QRegularExpression>

#include "src/helpers/_appContext.h"

class MessageInterpreter {
    public:
        enum Command { Unknown, Say, Whisper, Help };
        
        static inline QHash<QString, Command> _textByCommand = {
            { "/h", Help }
        };

        static Command interpretText(QString &text);

        static QList<QString> findRecipentsFromText(QString &text);

        static QString sanitizeText(QString text);

        static bool isSendable(QString &textToSend);

        static QString help();

        static QString usernameToCommandCompatible(QString username);

    private:
        static inline QRegularExpression _hasWhispRegex = QRegularExpression("@(\\w+)");
        static inline QRegularExpression _hasCommandRegex = QRegularExpression("\\/(\\w+)");
};