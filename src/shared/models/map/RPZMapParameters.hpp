#pragma once

#include <QVariantHash>

class RPZMapParameters : public QVariantHash {
    public:
        RPZMapParameters() {}
        explicit RPZMapParameters(const QVariantHash &hash) : QVariantHash(hash) {}
};