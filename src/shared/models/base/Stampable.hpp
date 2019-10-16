#pragma once

#include <QDateTime>
#include "Ownable.hpp"

#include <QPalette>

typedef snowflake_uid RPZStampableId;

class Stampable : public Ownable {
    public:
        Stampable() : Ownable(SnowFlake::get()->nextId()) {
            this->_setTimestamp(QDateTime::currentDateTime());
        }
        Stampable(const QVariantHash &hash) : Ownable(hash) {}

        QDateTime timestamp() const {
            return this->value(QStringLiteral(u"dt")).toDateTime();
        }

        virtual QString toString() const {
            const auto ts = this->timestamp().toString("dd.MM.yyyy-hh:mm:ss");
            const auto name = this->owner().name();
            return QStringLiteral(u"[%1] %2").arg(ts).arg(name);
        }

        virtual QPalette palette() const {
            QPalette palette;
            palette.setColor(QPalette::Window, "#fcfcfc");
            palette.setColor(QPalette::WindowText, "#999999");
            return palette;
        }

    private:
        void _setTimestamp(const QDateTime &dt) {
            this->insert(QStringLiteral(u"dt"), dt);
        }

};