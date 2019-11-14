#pragma once 

#include <QVariantHash>
#include <QMap>

class Compressible : protected QVariantHash {
    
    public:
        using Parameter = int;
        using KeyName = QString;

        Compressible() {};
        explicit Compressible(const QVariantHash &hash) : QVariantHash(hash) {}

        static inline QHash<Compressible::Parameter, Compressible::KeyName> model {};

};