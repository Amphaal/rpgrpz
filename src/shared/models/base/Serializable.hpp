#pragma once 

#include <QVariantHash>
#include <QMap>

#include "libs/snowflake.hpp"

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
            (*this)["id"] = QString::number(id);
        }
};

template<typename T>
class RPZMap : public QMap<snowflake_uid, T> {

    static_assert(std::is_base_of<Serializable, T>::value, "Must derive from Serializable");
    T element;
    
    public:
        RPZMap() {}
        RPZMap(T &singleAtom) {
            this->insert(singleAtom.id(), singleAtom);
        }

        QVariantList toVList() {
            QVariantList out;
            for(T &base : this->values()) {
                out.append(base);
            }
            return out;
        }
};

template<typename T>
class RPZList : public QList<T> {
    
    static_assert(std::is_base_of<Serializable, T>::value, "Must derive from Serializable");
    T element;
    
    public:
        RPZList(const QVariantList &list) {
            for(auto &elem : list) {
                auto casted = T(elem.toHash());
                this->append(casted);
            }
        }
};