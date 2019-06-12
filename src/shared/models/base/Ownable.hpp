#pragma once

#include <QString>

#include "Serializable.hpp"

#include "src/shared/models/RPZUser.hpp"

class Ownable : public Serializable {

    public:
        Ownable() {};
        Ownable(const QVariantHash &hash) : Serializable(hash) {}
        Ownable(const snowflake_uid &id) : Serializable(id) {}
        Ownable(const snowflake_uid &id, const RPZUser &user) : Ownable(id) {
            this->setOwnership(user);
        };

        RPZUser owner() { 
            return this->value("owner", RPZUser()).toHash(); 
        };

        void setOwnership(const RPZUser &user) { 
            this->insert("owner", user);
        };
};