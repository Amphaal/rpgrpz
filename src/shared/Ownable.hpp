#pragma once

#include <QUuid>
#include <QString>

#include "RPZUser.hpp"

class Ownable {

    public:
        Ownable() {};
        Ownable(const RPZUser &user) {
            this->setOwnership(user);
        };

        RPZUser owner() { return this->_owner; };

        void setOwnership(const RPZUser &user) { 
            this->_owner = user;
        };

    protected: 
        void injectOwnerDataToHash(QVariantHash &hash) {
            hash.insert("owner", this->_owner.toVariantHash());
        }

    private:
        RPZUser _owner;
};