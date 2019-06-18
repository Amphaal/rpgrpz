#pragma once 

#include <QVariantHash>
#include <QMap>

#include "src/_libs/snowflake.h"

class Serializable : public QVariantHash {
    
    public:
        Serializable() {};
        Serializable(const QVariantHash &hash) : QVariantHash(hash) {}
        
        Serializable(const snowflake_uid &id) {
            this->_setId(id);
        };

        snowflake_uid id() { 
            return this->value("id").toULongLong(); 
        };

        void shuffleId() {
            this->_setId(SnowFlake::get()->nextId());
        }
    
    private:
        void _setId(const snowflake_uid &id) {
            this->insert("id", QString::number(id));
        }
};