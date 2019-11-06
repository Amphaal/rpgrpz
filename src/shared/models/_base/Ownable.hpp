#pragma once

#include <QString>

#include "Serializable.hpp"

#include "src/shared/models/RPZUser.h"

class Ownable : public Serializable {

    public:
        Ownable() {};
        explicit Ownable(const QVariantHash &hash) : Serializable(hash) {}
        Ownable(SnowFlake::Id id) : Serializable(id) {}
        Ownable(SnowFlake::Id id, const RPZUser &user) : Ownable(id) {
            this->setOwnership(user);
        };

        RPZUser owner() const { 
            return RPZUser(this->value(QStringLiteral(u"owner")).toHash()); 
        };

        void setOwnership(const RPZUser &user) { 
            this->insert(QStringLiteral(u"owner"), user);
        };
};