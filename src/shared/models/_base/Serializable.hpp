#pragma once 

#include <QVariantHash>
#include <QMap>

#include "src/_libs/snowflake/snowflake.h"

class Serializable : public QVariantHash {
    
    public:
        Serializable() {};
        Serializable(const QVariantHash &hash) : QVariantHash(hash) {}
        
        Serializable(snowflake_uid id) {
            this->_setId(id);
        };

        snowflake_uid id() const { 
            return this->value(QStringLiteral(u"id")).toULongLong(); 
        };

        QString idAsStr() const { 
            return QString::number(this->id()); 
        };

        void shuffleId() {
            this->_setId(SnowFlake::get()->nextId());
        }
    
    private:
        void _setId(snowflake_uid id) {
            this->insert(QStringLiteral(u"id"), QString::number(id));
        }
};