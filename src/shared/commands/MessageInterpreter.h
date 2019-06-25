#pragma once

#include <QHash>
#include <QString>

#include <QRegularExpression>

#include "src/helpers/_appContext.h"

class MessageInterpreter {
    public:
        enum Command { Unknown, Say, Whisper, Help };
        
        static inline const QHash<QString, Command> _textByCommand = {
            { "/h", Help }
        };

        static Command interpretText(const QString &text);

        static QList<QString> findRecipentsFromText(const QString &text);

        static QString sanitizeText(const QString &text);

        static bool isSendable(const QString &textToSend);

        static QString help();

        static QString usernameToCommandCompatible(const QString &username);

    private:
        static inline const QRegularExpression _hasWhispRegex = QRegularExpression("@(\\w+)");
        static inline const QRegularExpression _hasCommandRegex = QRegularExpression("\\/(\\w+)");
};