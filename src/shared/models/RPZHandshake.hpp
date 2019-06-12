#pragma once

#include "src/helpers/_appContext.h"
#include <QVariantHash>

class RPZHandshake : public QVariantHash {
    public:
        RPZHandshake(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZHandshake(const QString &requestedUsername) {
            this->_setClientVersion();
            this->_setRequestedUsername(requestedUsername);
        }

        QString clientVersion() {
            return this->value("v").toString();
        }

        QString requestedUsername() {
            return this->value("un").toString();
        }

    private:
        void _setClientVersion() {
            this->insert("v", QString(APP_CURRENT_VERSION));
        }

        void _setRequestedUsername(const QString &requestedUsername) {
            this->insert("un", requestedUsername);
        }
};