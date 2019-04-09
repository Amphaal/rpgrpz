#pragma once

#include <QUuid>
#include <QString>

#include "RPZUser.hpp"

class Ownable {

    public:
        Ownable() {};
        Ownable(const QUuid &ownerId, const QString &ownerName) : _ownerId(ownerId), _ownerName(ownerName) {};
        Ownable(RPZUser* user) {
            this->setOwnership(user);
        };

        QUuid ownerId() { return this->_ownerId; };
        QString ownerName() { return this->_ownerName; };

        void setOwnership(RPZUser* user) { 
            if(user) {
                this->_ownerId = user->id();
                this->_ownerName = user->name();
            }
        };

    protected: 
        void injectOwnerDataToHash(QVariantHash &hash) {
            hash.insert("oid", this->ownerId());
            hash.insert("oname", this->ownerName());
        }

    private:
        QUuid _ownerId;
        QString _ownerName;
};