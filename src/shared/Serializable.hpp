#pragma once 

#include <QUuid>
#include <QVariantHash>

class Serializable {
    
    public:
        Serializable() {};
        Serializable(const QUuid &id) : _id(id) {};
        QUuid id() { return this->_id; };
        virtual QVariantHash toVariantHash() = 0;

    private:
        QUuid _id;
};