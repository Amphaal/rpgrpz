#pragma once

#include "Serializable.hpp"

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