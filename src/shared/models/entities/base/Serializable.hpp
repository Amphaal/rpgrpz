#pragma once 

#include <QUuid>
#include <QVariantHash>

class Serializable : public QVariantHash {
    
    public:
        Serializable() {};
        Serializable(const QVariantHash &hash) : QVariantHash(hash) {}
        Serializable(const QUuid &id) {
            (*this)["id"] = id;
        };
        QUuid id() { 
            return this->value("id").toUuid(); 
        };

    private:
        QUuid _id;
};