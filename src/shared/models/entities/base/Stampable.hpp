#pragma once

#include <QDateTime>
#include "Ownable.hpp"

class Stampable : public Ownable {
    public:
        Stampable() : Ownable(QUuid::createUuid()) {
            this->_setTimestamp(QDateTime::currentDateTime());
        }
        Stampable(const QVariantHash &hash) : Ownable(hash) {}

        QDateTime timestamp() {
            return this->value("dt").toDateTime();
        }


        virtual QString toString() {
            const auto ts = QString("[" + this->timestamp().toString("dd.MM.yyyy-hh:mm:ss") + "] ");
            const auto name = this->owner().name();
            return ts + name;
        }

    private:
        void _setTimestamp(const QDateTime &dt) {
            (*this)["dt"] = dt;
        }

};