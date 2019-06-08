#include "MessageInterpreter.h"

MessageInterpreter::Command MessageInterpreter::interpretText(QString &text) {
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

QList<QString> MessageInterpreter::findRecipentsFromText(QString &text) {
    
    auto matches = _hasWhispRegex.globalMatch(text);
    QSet<QString> out;

    //iterate
    while (matches.hasNext()) {
        auto match = matches.next(); //next
        out.insert(match.captured(1));
    }

    return out.toList();
}

QString MessageInterpreter::sanitizeText(QString text) {
    text.remove(_hasWhispRegex);
    text.remove(_hasCommandRegex);
    return text.trimmed();
}

bool MessageInterpreter::isSendable(QString &textToSend) {
    
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

QString MessageInterpreter::usernameToCommandCompatible(QString username) {
    return username.toLower().replace(" ", "_");
}
