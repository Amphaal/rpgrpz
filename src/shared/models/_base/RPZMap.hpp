#pragma once

#include <QList>

#include "Serializable.hpp"

template<class T>
class RPZMap : public QMap<SnowFlake::Id, T> {

    static_assert(std::is_base_of<Serializable, T>::value, "Must derive from Serializable");
    T element;
    
    public:
        RPZMap() {}    
        ~RPZMap() {}   

        RPZMap(const QList<T> &serializables) {
            for(const auto &s : serializables) {
                this->insert(s.id(), s);
            }
        }

        RPZMap(const T &serializable) {
            this->insert(serializable.id(), serializable);
        }

        QVariantList toVList() const {
            QVariantList out;
            for(const auto &base : this->values()) {
                out.append(base);
            }
            return out;
        }

        QVariantMap toVMap() const {
            QVariantMap out;
            for(auto i = this->begin(); i != this->end(); i++) {
                out.insert(QString::number(i.key()), i.value());
            }
            return out;
        }

        static RPZMap<T> fromVMap(const QVariantMap &map) {
            
            RPZMap<T> out;
            
            for(auto i = map.constBegin(); i != map.constEnd(); i++) {
                auto id = i.key().toULongLong();
                T casted(i.value().toHash());
                out.insert(id, casted);
            }

            return out;

        }

};