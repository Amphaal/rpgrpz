#pragma once

#include "Serializable.hpp"
#include <QList>

template<typename T>
class RPZMap : public QMap<snowflake_uid, T> {

    static_assert(std::is_base_of<Serializable, T>::value, "Must derive from Serializable");
    T element;
    
    public:
        RPZMap() {}
        RPZMap(const QList<T> &serializables) {
            for(auto &s : serializables) {
                this->insert(s.id(), s);
            }
        }

        RPZMap(const T &serializable) {
            this->insert(serializable.id(), serializable);
        }

        QVariantList toVList() const {
            QVariantList out;
            for(T &base : this->values()) {
                out.append(base);
            }
            return out;
        }
};