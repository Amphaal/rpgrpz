#pragma once

#include <QHash>
#include <QString>

#include <QRegularExpression>
#include <QRandomGenerator>
#include <QPair>

#include "src/helpers/_appContext.h"

struct DiceThrow {
    uint face = 0;
    uint howMany = 0;
    QVector<uint> values;
    QVector<QPair<uint, int>> pairedValues;
    QString name;
    double avg = 0;
};

class MessageInterpreter {
    public:
        enum Command { C_Unknown, Say, Whisper, Help, C_DiceThrow };
        
        static inline const QHash<QString, Command> _textByCommand = {
            { "/h", Help }
        };

        static Command interpretText(const QString &text);

        static QList<QString> findRecipentsFromText(const QString &text);

        static QString sanitizeText(const QString &text);

        static bool isSendable(const QString &textToSend);

        static QString help();

        static QString usernameToCommandCompatible(const QString &username);

        static QVector<DiceThrow> findDiceThrowsFromText(const QString &text);
        static void generateValuesOnDiceThrows(QVector<DiceThrow> &throws);

    private:
        static inline const QRegularExpression _hasWhispRegex = QRegularExpression("@(.+?)(?:$|\\s)");
        static inline const QRegularExpression _hasCommandRegex = QRegularExpression("\\/(\\w+)");
        static inline const QRegularExpression _mustLaunchDice = QRegularExpression("([1-9]|1[0-6])[dD](\\d+)");
};