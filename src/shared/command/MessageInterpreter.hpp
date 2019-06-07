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

        static Command interpretText(QString &text) {
            //initial trim
            text = text.trimmed();
            
            //if has whisper command
            if(_hasWhispRegex.match(text).hasMatch()) return Whisper;

            //if not a command
            if(!text.startsWith("/")) return Say;

            //extract command
            auto command = text.split(" ", QString::SplitBehavior::SkipEmptyParts)[0];

            //returns command   
            return _textByCommand[command];
        }

        static QList<QString> findRecipentsFromText(QString &text) {
            
            auto matches = _hasWhispRegex.globalMatch(text);
            QSet<QString> out;

            //iterate
            while (matches.hasNext()) {
                auto match = matches.next(); //next
                out.insert(match.captured(1));
            }

            return out.toList();
        }

        static QString sanitizeText(QString text) {
            text.remove(_hasWhispRegex);
            text.remove(_hasCommandRegex);
            return text.trimmed();
        }

        static bool isSendable(QString &textToSend) {
            
            //if has whisper
            if(_hasWhispRegex.match(textToSend).hasMatch()) {
                auto q = sanitizeText(textToSend);
                return !q.isEmpty();
            }

            return !textToSend.trimmed().isEmpty();
        }

        static QString help()  {
            QString help;
            help += QString("Serveur ") + QString(APP_FULL_DENOM) + " :\n";
            help += "- Pour chuchotter : @{nom}. Vous pouvez chuchotter à plusieurs personne pour 1 message.";
            return help;
        }

        static QString usernameToCommandCompatible(QString username) {
            return username.toLower().replace(" ", "_");
        }

    private:
        static inline QRegularExpression _hasWhispRegex = QRegularExpression("@(\\w+)");
        static inline QRegularExpression _hasCommandRegex = QRegularExpression("\\/(\\w+)");
};