#pragma once 

#include <QVariantHash>
#include <QMap>

class Compressible : protected QVariantHash {
    
    public:
        typedef int Parameter;
        typedef QString KeyName;

        Compressible() {};
        explicit Compressible(const QVariantHash &hash) : QVariantHash(hash) {}

        static inline QHash<Compressible::Parameter, Compressible::KeyName> model {};

};