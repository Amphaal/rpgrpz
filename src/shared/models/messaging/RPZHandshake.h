#pragma once

#include "src/helpers/_appContext.h"
#include <QVariantHash>

#include "src/shared/models/character/RPZCharacter.hpp"

class RPZHandshake : public QVariantHash {
    public:
        RPZHandshake(const QVariantHash &hash);
        RPZHandshake(const QString &requestedUsername, const RPZCharacter &toIncarnate);

        const QString clientVersion() const;
        const QString requestedUsername() const;
        const RPZCharacter incarnatingAs() const;

};