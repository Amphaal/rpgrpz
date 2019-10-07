#include "RPZHandshake.h"

RPZHandshake::RPZHandshake(const QVariantHash &hash) : QVariantHash(hash) {}
RPZHandshake::RPZHandshake(const QString &requestedUsername, const RPZCharacter &toIncarnate) {
    this->insert("v", QString(APP_CURRENT_VERSION));
    this->insert("un", requestedUsername);
    this->insert("char", toIncarnate);
}

const QString RPZHandshake::clientVersion() const {
    return this->value("v").toString();
}

const QString RPZHandshake::requestedUsername() const {
    return this->value("un").toString();
}

const RPZCharacter RPZHandshake::incarnatingAs() const {
    return RPZCharacter(this->value("char").toHash());
}