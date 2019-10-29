#pragma once

#include <QList>

#include "Serializable.hpp"

template<class T>
class RPZMap : public QMap<SnowFlake::Id, T> {

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

        QVariantMap toVMap() const {
            QVariantMap out;
            for(auto i = this->begin(); i != this->end(); i++) {
                out.insert(QString::number(i.key()), i.value());
            }
            return out;
        }
};