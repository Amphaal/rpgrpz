#pragma once 

#include <QUuid>
#include <QVariantHash>

class Serializable : public QVariantHash {
    
    public:
        Serializable() {};
        Serializable(const QVariantHash &hash) : QVariantHash(hash) {}
        Serializable(const QUuid &id) {
            (*this)["id"] = id;
        };
        QUuid id() { 
            return this->value("id").toUuid(); 
        };

    private:
        QUuid _id;
};

template<typename T>
class RPZHash : public QHash<QUuid, T> {

    static_assert(std::is_base_of<Serializable, T>::value, "Must derive from Serializable");
    T element;
    
    public:
        QVariantList toVList() {
            QVariantList out;
            for(T &base : this->values()) {
                out.append(base);
            }
            return out;
        }
};