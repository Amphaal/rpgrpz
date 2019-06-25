#include "MessageInterpreter.h"

MessageInterpreter::Command MessageInterpreter::interpretText(const QString &text) {
    //initial trim
    auto cp_text = text;
    cp_text = cp_text.trimmed();
    
    //if has whisper command
    if(_hasWhispRegex.match(cp_text).hasMatch()) return Whisper;

    //if not a command
    if(!cp_text.startsWith("/")) return Say;

    //extract command
    auto command = cp_text.split(" ", QString::SplitBehavior::SkipEmptyParts)[0];

    //returns command   
    return _textByCommand[command];
}

QList<QString> MessageInterpreter::findRecipentsFromText(const QString &text) {
    
    auto matches = _hasWhispRegex.globalMatch(text);
    QSet<QString> out;

    //iterate
    while (matches.hasNext()) {
        auto match = matches.next(); //next
        out.insert(match.captured(1));
    }

    return out.toList();
}

QString MessageInterpreter::sanitizeText(const QString &text) {
    auto cp_text = text;
    cp_text.remove(_hasWhispRegex);
    cp_text.remove(_hasCommandRegex);
    return cp_text.trimmed();
}

bool MessageInterpreter::isSendable(const QString &textToSend) {
    
    //if has whisper
    if(_hasWhispRegex.match(textToSend).hasMatch()) {
        auto q = sanitizeText(textToSend);
        return !q.isEmpty();
    }

    return !textToSend.trimmed().isEmpty();
}

QString MessageInterpreter::help()  {
    QString help;
    help += QString("Serveur ") + QString(APP_FULL_DENOM) + " :\n";
    help += "- Pour chuchotter : @{nom}. Vous pouvez chuchotter à plusieurs personne pour 1 message.";
    return help;
}

QString MessageInterpreter::usernameToCommandCompatible(const QString &username) {
    auto cp_username = username;
    cp_username = cp_username.toLower();
    cp_username = cp_username.replace(" ", "_");
    return cp_username;
}
