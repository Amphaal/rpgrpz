#pragma once

#include <QUuid>
#include <QString>

#include "Serializable.hpp"

#include "RPZUser.hpp"

class Ownable : public Serializable {

    public:
        Ownable() {};
        Ownable(const QVariantHash &hash) : Serializable(hash) {}
        Ownable(const QUuid &id) : Serializable(id) {}
        Ownable(const QUuid &id, const RPZUser &user) : Ownable(id) {
            this->setOwnership(user);
        };

        RPZUser owner() { 
            return this->value("owner").toHash(); 
        };

        void setOwnership(const RPZUser &user) { 
            (*this)["owner"] = user;
        };
};