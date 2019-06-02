#pragma once 

#include <QString>
#include <QUuid>
#include <QDateTime>

#include "RPZUser.hpp"
#include "base/Ownable.hpp"

class RPZMessage : public Ownable {
    public:
        RPZMessage() {};
        RPZMessage(const QVariantHash &hash) : Ownable(hash) {}

        RPZMessage(const QString &message) : Ownable(QUuid::createUuid()) { 
            this->_setMessage(message);
            this->_setTimestamp(QDateTime::currentDateTime());
        };

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

        QUuid respondTo() {
            return this->value("r_id").toUuid();
        }

        void setResponseToMessageId(const QUuid &answerMessageId) {
            (*this)["r_id"] = answerMessageId;
        }

    private:
        void _setTimestamp(const QDateTime &dt) {
            (*this)["dt"] = dt;
        }

        void _setMessage(const QString &message) {
            (*this)["msg"] = message;
        }
};