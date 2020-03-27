#pragma once 

#include <QVariantHash>
#include <QMap>

#include "src/_libs/snowflake/snowflake.h"

class Serializable : public QVariantHash {
    
    public:
        Serializable() {};
        explicit Serializable(const QVariantHash &hash) : QVariantHash(hash) {}
        
        Serializable(const SnowFlake::Id &id) {
            this->_setId(id);
        };

        SnowFlake::Id id() const { 
            return this->value(QStringLiteral(u"id")).toULongLong(); 
        };

        QString idAsStr() const { 
            return QString::number(this->id()); 
        };

        void shuffleId() {
            auto id = SnowFlake::get()->nextId();
            this->_setId(id);
        }
    
    private:
        void _setId(const SnowFlake::Id &id) {
            this->insert(QStringLiteral(u"id"), QVariant::fromValue<SnowFlake::Id>(id));
        }
};