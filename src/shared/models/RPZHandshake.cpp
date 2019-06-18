#include "RPZHandshake.h"

RPZHandshake::RPZHandshake(const QVariantHash &hash) : QVariantHash(hash) {}
RPZHandshake::RPZHandshake(const QString &requestedUsername) {
    this->_setClientVersion();
    this->_setRequestedUsername(requestedUsername);
}

QString RPZHandshake::clientVersion() {
    return this->value("v").toString();
}

QString RPZHandshake::requestedUsername() {
    return this->value("un").toString();
}

void RPZHandshake::_setClientVersion() {
    this->insert("v", QString(APP_CURRENT_VERSION));
}

void RPZHandshake::_setRequestedUsername(const QString &requestedUsername) {
    this->insert("un", requestedUsername);
}