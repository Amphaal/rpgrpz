#pragma once 

#include <QVariantHash>
#include <QMap>

#include "src/_libs/snowflake/snowflake.h"

class Serializable : public QVariantHash {
    
    public:
        Serializable() {};
        explicit Serializable(const QVariantHash &hash) : QVariantHash(hash) {}
        
        Serializable(SnowFlake::Id id) {
            this->_setId(id);
        };

        SnowFlake::Id id() const { 
            return this->value(QStringLiteral(u"id")).toULongLong(); 
        };

        QString idAsStr() const { 
            return QString::number(this->id()); 
        };

        void shuffleId() {
            this->_setId(SnowFlake::get()->nextId());
        }
    
    private:
        void _setId(SnowFlake::Id id) {
            this->insert(QStringLiteral(u"id"), QString::number(id));
        }
};