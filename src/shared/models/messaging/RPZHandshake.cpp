#include "RPZHandshake.h"

RPZHandshake::RPZHandshake(const QVariantHash &hash) : QVariantHash(hash) {}
RPZHandshake::RPZHandshake(const QString &requestedUsername, const RPZCharacter &toIncarnate) {
    this->insert(QStringLiteral(u"v"), QString(APP_CURRENT_VERSION));
    this->insert(QStringLiteral(u"un"), requestedUsername);
    this->insert(QStringLiteral(u"char"), toIncarnate);
}

const QString RPZHandshake::clientVersion() const {
    return this->value(QStringLiteral(u"v")).toString();
}

const QString RPZHandshake::requestedUsername() const {
    return this->value(QStringLiteral(u"un")).toString();
}

const RPZCharacter RPZHandshake::incarnatingAs() const {
    return RPZCharacter(this->value(QStringLiteral(u"char")).toHash());
}