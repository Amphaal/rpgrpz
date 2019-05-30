#pragma once 

#include <QString>
#include <QUuid>
#include <QDateTime>

#include "RPZUser.hpp"
#include "Ownable.hpp"

class RPZMessage : public Ownable {
    public:
        RPZMessage() {};
        RPZMessage(const QVariantHash &hash) : Ownable(hash) {}
        RPZMessage(const QUuid &id, const QString &message, const QDateTime &dt, const RPZUser &owner) : Ownable(id, owner) {
            this->_setTimestamp(dt);
            this->_setMessage(message);
        };

        RPZMessage(const QUuid &id, const QString &message, const QDateTime &dt) : Ownable(id) { 
            this->_setTimestamp(dt);
            this->_setMessage(message);
        };

        RPZMessage(const QString &message) : 
        RPZMessage(
            QUuid::createUuid(), 
            message, 
            QDateTime::currentDateTime()
        ) { };

        QString message() {
            return this->value("msg").toString();
        }

        QDateTime timestamp() {
            return this->value("dt").toDateTime();
        }

        QString toString() {
            const auto ts = QString("[" + this->timestamp().toString("dd.MM.yyyy-hh:mm:ss") + "] ");
            const auto name = this->owner().name() + " a dit : ";
            const auto fullMsg = ts + name + "“" + this->message() + "”";
            return fullMsg;
        };


    private:
        void _setTimestamp(const QDateTime &dt) {
            (*this)["dt"] = dt;
        }

        void _setMessage(const QString &message) {
            (*this)["msg"] = message;
        }
};