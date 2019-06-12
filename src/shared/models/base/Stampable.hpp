#pragma once

#include <QDateTime>
#include "Ownable.hpp"

#include <QPalette>

class Stampable : public Ownable {
    public:
        Stampable() : Ownable(SnowFlake::get()->nextId()) {
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

        virtual QPalette palette() {
            QPalette palette;
            palette.setColor(QPalette::Window, "#fcfcfc");
            palette.setColor(QPalette::WindowText, "#999999");
            return palette;
        }

    private:
        void _setTimestamp(const QDateTime &dt) {
            this->insert("dt", dt);
        }

};