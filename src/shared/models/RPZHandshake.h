#pragma once

#include "src/helpers/_appContext.h"
#include <QVariantHash>

class RPZHandshake : public QVariantHash {
    public:
        RPZHandshake(const QVariantHash &hash);
        RPZHandshake(const QString &requestedUsername);

        QString clientVersion();
        QString requestedUsername();

    private:
        void _setClientVersion();
        void _setRequestedUsername(const QString &requestedUsername);
};