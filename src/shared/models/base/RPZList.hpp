#pragma once

#include "Serializable.hpp"

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